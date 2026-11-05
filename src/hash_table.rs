// Хеш-таблицы «ключ -> значение»: метод цепочек и открытая адресация.
use crate::codec;
use crate::container::{Container, LoadError, Serializable};
use std::io::{self, BufRead, Read, Write};

// Полиномиальная хеш-функция строки: h = h * 31 + байт (mod 2^64)
pub fn hash_string(text: &str) -> u64 {
    text.bytes()
        .fold(0u64, |h, b| h.wrapping_mul(31).wrapping_add(u64::from(b)))
}

// Общий интерфейс хеш-таблиц; сериализация пар написана один раз для всех реализаций
pub trait HashTable: Container {
    // Вставка или замена; true, если ключ новый
    fn put(&mut self, key: &str, value: &str) -> bool;
    fn get(&self, key: &str) -> Option<&str>;
    fn remove(&mut self, key: &str) -> bool;
    fn for_each_pair(&self, visit: &mut dyn FnMut(&str, &str));
    fn capacity(&self) -> usize;

    fn contains(&self, key: &str) -> bool {
        self.get(key).is_some()
    }

    #[allow(clippy::cast_precision_loss)]
    fn load_factor(&self) -> f64 {
        self.length() as f64 / self.capacity() as f64
    }

    fn pairs(&self) -> Vec<(String, String)> {
        let mut out = Vec::new();
        self.for_each_pair(&mut |k, v| out.push((k.to_string(), v.to_string())));
        out.sort();
        out
    }
}

fn save_pairs_binary<T: HashTable + ?Sized>(t: &T, out: &mut dyn Write) -> io::Result<()> {
    codec::write_binary_header(out, t.type_name(), t.length())?;
    let mut result = Ok(());
    t.for_each_pair(&mut |k, v| {
        if result.is_ok() {
            result = codec::write_string(out, k).and_then(|()| codec::write_string(out, v));
        }
    });
    result
}

fn load_pairs_binary<T: HashTable + ?Sized>(
    t: &mut T,
    input: &mut dyn Read,
) -> Result<(), LoadError> {
    let count = codec::read_binary_header(input, t.type_name())?;
    let mut pairs = Vec::new();
    for _ in 0..count {
        let k = codec::read_string(input)?;
        let v = codec::read_string(input)?;
        pairs.push((k, v));
    }
    t.clear();
    for (k, v) in &pairs {
        t.put(k, v);
    }
    Ok(())
}

// Текстовый формат - JSON из ЛР №1: ключи и значения идут по очереди ["k1", "v1", "k2", "v2"]
fn save_pairs_text<T: HashTable + ?Sized>(t: &T, out: &mut dyn Write) -> io::Result<()> {
    let mut flat = Vec::new();
    t.for_each_pair(&mut |k, v| {
        flat.push(k.to_string());
        flat.push(v.to_string());
    });
    codec::write_json(out, t.type_name(), &flat)
}

fn load_pairs_text<T: HashTable + ?Sized>(
    t: &mut T,
    input: &mut dyn BufRead,
) -> Result<(), LoadError> {
    let flat = codec::read_json(input, t.type_name())?;
    if flat.len() % 2 != 0 {
        return Err(LoadError::Format("expected pairs key, value".into()));
    }
    t.clear();
    for pair in flat.chunks(2) {
        t.put(&pair[0], &pair[1]);
    }
    Ok(())
}

macro_rules! impl_table_serializable {
    ($t:ty) => {
        impl Serializable for $t {
            fn save_binary(&self, out: &mut dyn Write) -> io::Result<()> {
                save_pairs_binary(self, out)
            }
            fn load_binary(&mut self, input: &mut dyn Read) -> Result<(), LoadError> {
                load_pairs_binary(self, input)
            }
            fn save_text(&self, out: &mut dyn Write) -> io::Result<()> {
                save_pairs_text(self, out)
            }
            fn load_text(&mut self, input: &mut dyn BufRead) -> Result<(), LoadError> {
                load_pairs_text(self, input)
            }
        }
    };
}

// ---------- Метод цепочек ----------

#[derive(Debug, Clone)]
struct Entry {
    key: String,
    value: String,
    next: Option<Box<Entry>>,
}

// Ячейка - односвязная цепочка пар; при заполнении > 0,75 число ячеек удваивается
#[derive(Debug, Clone)]
pub struct ChainHashTable {
    buckets: Vec<Option<Box<Entry>>>,
    count: usize,
}

const START_BUCKETS: usize = 8;

impl Default for ChainHashTable {
    fn default() -> Self {
        Self::with_buckets(START_BUCKETS)
    }
}

impl ChainHashTable {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn with_buckets(n: usize) -> Self {
        ChainHashTable {
            buckets: (0..n.max(1)).map(|_| None).collect(),
            count: 0,
        }
    }

    #[allow(clippy::cast_possible_truncation)]
    fn index(&self, key: &str) -> usize {
        (hash_string(key) % self.buckets.len() as u64) as usize
    }

    // Перехеширование: номер ячейки зависит от их числа, поэтому все пары раскладываются заново
    fn rehash(&mut self, new_count: usize) {
        let old = std::mem::replace(&mut self.buckets, (0..new_count).map(|_| None).collect());
        for mut chain in old {
            while let Some(mut e) = chain {
                chain = e.next.take();
                let i = self.index(&e.key);
                e.next = self.buckets[i].take();
                self.buckets[i] = Some(e);
            }
        }
    }

    pub fn longest_chain(&self) -> usize {
        self.buckets
            .iter()
            .map(|b| std::iter::successors(b.as_deref(), |e| e.next.as_deref()).count())
            .max()
            .unwrap_or(0)
    }
}

impl Container for ChainHashTable {
    fn length(&self) -> usize {
        self.count
    }
    fn clear(&mut self) {
        *self = Self::default();
    }
    fn type_name(&self) -> &'static str {
        "ChainHashTable"
    }
}

impl HashTable for ChainHashTable {
    fn put(&mut self, key: &str, value: &str) -> bool {
        let i = self.index(key);
        let mut cur = self.buckets[i].as_deref_mut();
        while let Some(e) = cur {
            if e.key == key {
                e.value = value.to_string();
                return false;
            }
            cur = e.next.as_deref_mut();
        }
        if (self.count + 1) * 4 > self.buckets.len() * 3 {
            self.rehash(self.buckets.len() * 2);
        }
        let i = self.index(key);
        let next = self.buckets[i].take();
        self.buckets[i] = Some(Box::new(Entry {
            key: key.to_string(),
            value: value.to_string(),
            next,
        }));
        self.count += 1;
        true
    }

    fn get(&self, key: &str) -> Option<&str> {
        std::iter::successors(self.buckets[self.index(key)].as_deref(), |e| {
            e.next.as_deref()
        })
        .find(|e| e.key == key)
        .map(|e| e.value.as_str())
    }

    fn remove(&mut self, key: &str) -> bool {
        let i = self.index(key);
        let mut link = &mut self.buckets[i];
        loop {
            match link {
                None => return false,
                Some(e) if e.key == key => {
                    *link = e.next.take();
                    self.count -= 1;
                    return true;
                }
                Some(e) => link = &mut e.next,
            }
        }
    }

    fn for_each_pair(&self, visit: &mut dyn FnMut(&str, &str)) {
        for b in &self.buckets {
            for e in std::iter::successors(b.as_deref(), |e| e.next.as_deref()) {
                visit(&e.key, &e.value);
            }
        }
    }

    fn capacity(&self) -> usize {
        self.buckets.len()
    }
}

impl_table_serializable!(ChainHashTable);

// ---------- Открытая адресация ----------

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Probing {
    Linear,
    Double,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Cell {
    Empty,
    Busy,
    // «Надгробие»: ячейка освобождена, но поиск через неё продолжается
    Deleted,
}

#[derive(Debug, Clone)]
pub struct OpenHashTable {
    probing: Probing,
    keys: Vec<String>,
    values: Vec<String>,
    cells: Vec<Cell>,
    count: usize,
    deleted: usize,
}

pub fn is_prime(n: usize) -> bool {
    if n < 2 {
        return false;
    }
    let mut d = 2;
    while d * d <= n {
        if n.is_multiple_of(d) {
            return false;
        }
        d += 1;
    }
    true
}

pub fn next_prime(mut n: usize) -> usize {
    while !is_prime(n) {
        n += 1;
    }
    n
}

impl Default for OpenHashTable {
    fn default() -> Self {
        Self::new(Probing::Double, 11)
    }
}

impl OpenHashTable {
    // Размер округляется до простого: тогда любой шаг h2 обходит все ячейки
    pub fn new(probing: Probing, size: usize) -> Self {
        let m = next_prime(size.max(3));
        OpenHashTable {
            probing,
            keys: vec![String::new(); m],
            values: vec![String::new(); m],
            cells: vec![Cell::Empty; m],
            count: 0,
            deleted: 0,
        }
    }

    pub fn probing(&self) -> Probing {
        self.probing
    }

    // i-я проба: (h1 + i) mod m или (h1 + i * h2) mod m, где h2 = 1 + H mod (m - 1)
    #[allow(clippy::cast_possible_truncation)]
    fn probe(&self, hash: u64, i: usize) -> usize {
        let m = self.cells.len() as u64;
        let step = match self.probing {
            Probing::Linear => 1,
            Probing::Double => 1 + hash % (m - 1),
        };
        ((hash % m + (i as u64) * step) % m) as usize
    }

    // Номер ячейки ключа (если есть) и число просмотренных ячеек
    fn find(&self, key: &str) -> (Option<usize>, usize) {
        let h = hash_string(key);
        for i in 0..self.cells.len() {
            let pos = self.probe(h, i);
            match self.cells[pos] {
                Cell::Empty => return (None, i + 1),
                Cell::Busy if self.keys[pos] == key => return (Some(pos), i + 1),
                _ => {}
            }
        }
        (None, self.cells.len())
    }

    pub fn probes(&self, key: &str) -> usize {
        self.find(key).1
    }

    fn insert_new(&mut self, key: &str, value: &str) {
        let h = hash_string(key);
        for i in 0..self.cells.len() {
            let pos = self.probe(h, i);
            if self.cells[pos] != Cell::Busy {
                if self.cells[pos] == Cell::Deleted {
                    self.deleted -= 1;
                }
                self.keys[pos] = key.to_string();
                self.values[pos] = value.to_string();
                self.cells[pos] = Cell::Busy;
                self.count += 1;
                return;
            }
        }
        unreachable!("table is rebuilt before it becomes full");
    }

    fn rebuild(&mut self, size: usize) {
        let old = std::mem::replace(self, Self::new(self.probing, size));
        for (i, cell) in old.cells.iter().enumerate() {
            if *cell == Cell::Busy {
                self.insert_new(&old.keys[i], &old.values[i]);
            }
        }
    }
}

impl Container for OpenHashTable {
    fn length(&self) -> usize {
        self.count
    }
    fn clear(&mut self) {
        *self = Self::new(self.probing, 11);
    }
    fn type_name(&self) -> &'static str {
        match self.probing {
            Probing::Linear => "OpenHashTable/linear",
            Probing::Double => "OpenHashTable/double",
        }
    }
}

impl HashTable for OpenHashTable {
    fn put(&mut self, key: &str, value: &str) -> bool {
        if let (Some(pos), _) = self.find(key) {
            self.values[pos] = value.to_string();
            return false;
        }
        let m = self.cells.len();
        // Заполнено (вместе с надгробиями) больше 75 %: увеличить таблицу или просто убрать надгробия
        if (self.count + self.deleted + 1) * 4 > m * 3 {
            self.rebuild(if (self.count + 1) * 2 > m { m * 2 } else { m });
        }
        self.insert_new(key, value);
        true
    }

    fn get(&self, key: &str) -> Option<&str> {
        self.find(key).0.map(|pos| self.values[pos].as_str())
    }

    fn remove(&mut self, key: &str) -> bool {
        let Some(pos) = self.find(key).0 else {
            return false;
        };
        self.cells[pos] = Cell::Deleted;
        self.keys[pos].clear();
        self.values[pos].clear();
        self.count -= 1;
        self.deleted += 1;
        true
    }

    fn for_each_pair(&self, visit: &mut dyn FnMut(&str, &str)) {
        for (i, cell) in self.cells.iter().enumerate() {
            if *cell == Cell::Busy {
                visit(&self.keys[i], &self.values[i]);
            }
        }
    }

    fn capacity(&self) -> usize {
        self.cells.len()
    }
}

impl_table_serializable!(OpenHashTable);

#[cfg(test)]
mod tests {
    use super::*;

    fn tables() -> Vec<Box<dyn HashTable>> {
        vec![
            Box::new(ChainHashTable::new()),
            Box::new(OpenHashTable::new(Probing::Linear, 11)),
            Box::new(OpenHashTable::new(Probing::Double, 11)),
        ]
    }

    #[test]
    fn hash_function() {
        assert_eq!(hash_string(""), 0);
        assert_eq!(hash_string("a"), 97);
        assert_eq!(hash_string("ab"), 97 * 31 + 98);
        assert_ne!(hash_string("ab"), hash_string("ba"));
    }

    #[test]
    fn put_get_remove_all_kinds() {
        for mut t in tables() {
            assert!(t.is_empty() && t.get("x").is_none() && !t.remove("x"));
            assert!(t.put("apple", "1"));
            assert!(t.put("pear", "2"));
            assert!(!t.put("apple", "10"));
            assert_eq!(t.get("apple"), Some("10"));
            assert!(t.contains("pear") && !t.contains("plum"));
            assert_eq!(t.length(), 2);
            assert!(t.load_factor() > 0.0);
            assert!(t.remove("pear") && !t.remove("pear"));
            assert_eq!(t.pairs(), vec![("apple".to_string(), "10".to_string())]);
            t.clear();
            assert!(t.is_empty());
        }
    }

    #[test]
    fn grows_and_keeps_all_keys() {
        for mut t in tables() {
            let start = t.capacity();
            for i in 0..2000 {
                t.put(&format!("key{i}"), &i.to_string());
            }
            assert_eq!(t.length(), 2000);
            assert!(t.capacity() > start && t.load_factor() <= 0.75);
            assert!((0..2000).all(|i| t.get(&format!("key{i}")) == Some(i.to_string().as_str())));
            for i in (0..2000).step_by(2) {
                t.remove(&format!("key{i}"));
            }
            assert_eq!(t.length(), 1000);
            assert!(!t.contains("key0") && t.contains("key1"));
        }
    }

    #[test]
    fn chain_specifics() {
        let mut t = ChainHashTable::with_buckets(4);
        assert_eq!((t.capacity(), t.type_name()), (4, "ChainHashTable"));
        for k in ["a", "b", "c", "d", "e"] {
            t.put(k, k);
        }
        assert!(t.capacity() >= 8 && t.longest_chain() >= 1);
        assert_eq!(ChainHashTable::with_buckets(0).capacity(), 1);
        let copy = t.clone();
        t.clear();
        assert_eq!(copy.length(), 5);
    }

    #[test]
    fn open_addressing_tombstones() {
        let mut lin = OpenHashTable::new(Probing::Linear, 11);
        let mut dbl = OpenHashTable::new(Probing::Double, 11);
        assert_eq!(
            (lin.type_name(), dbl.type_name()),
            ("OpenHashTable/linear", "OpenHashTable/double")
        );
        assert_eq!(lin.probing(), Probing::Linear);
        for t in [&mut lin, &mut dbl] {
            for k in ["pear", "lime", "grape"] {
                t.put(k, k);
            }
            assert_eq!(t.capacity(), 11);
            assert!(t.remove("lime"));
            assert_eq!(t.get("grape"), Some("grape"));
            assert!(t.put("lime", "again"));
            assert_eq!(t.get("lime"), Some("again"));
        }
        assert_eq!((lin.probes("grape"), dbl.probes("grape")), (3, 2));
    }

    #[test]
    fn open_helpers_and_tombstone_cleanup() {
        assert!(is_prime(2) && is_prime(1009) && !is_prime(1) && !is_prime(1000));
        assert_eq!(next_prime(1000), 1009);
        assert_eq!(OpenHashTable::new(Probing::Linear, 1).capacity(), 3);
        let mut t = OpenHashTable::default();
        for i in 0..30 {
            t.put(&format!("k{i}"), "v");
            t.remove(&format!("k{i}"));
        }
        assert_eq!((t.length(), t.capacity()), (0, 11));
        assert!(t.put("alive", "1"));
        assert!(t.probes("missing") >= 1);
    }
}
