use crate::container::{impl_sequence_serializable, Container, Sequence};

// Двусвязный список. Узлы хранятся в векторе nodes,
// связи prev и next - номера узлов.
struct Node {
    value: String,
    prev: Option<usize>,
    next: Option<usize>,
}

pub struct DoublyList {
    nodes: Vec<Option<Node>>,
    free: Vec<usize>,
    head: Option<usize>,
    tail: Option<usize>,
    count: usize,
}

impl DoublyList {
    pub fn new() -> DoublyList {
        DoublyList {
            nodes: Vec::new(),
            free: Vec::new(),
            head: None,
            tail: None,
            count: 0,
        }
    }

    fn node(&self, i: usize) -> &Node {
        self.nodes[i].as_ref().unwrap()
    }

    fn node_mut(&mut self, i: usize) -> &mut Node {
        self.nodes[i].as_mut().unwrap()
    }

    fn create(&mut self, value: &str, prev: Option<usize>, next: Option<usize>) -> usize {
        let node = Node {
            value: value.to_string(),
            prev,
            next,
        };
        self.count += 1;
        if let Some(i) = self.free.pop() {
            self.nodes[i] = Some(node);
            i
        } else {
            self.nodes.push(Some(node));
            self.nodes.len() - 1
        }
    }

    // Поиск узла по значению
    fn find(&self, key: &str) -> Option<usize> {
        let mut current = self.head;
        while let Some(i) = current {
            if self.node(i).value == key {
                return Some(i);
            }
            current = self.node(i).next;
        }
        None
    }

    // Исключить узел из списка и вернуть его значение
    fn unlink(&mut self, i: usize) -> String {
        let node = self.nodes[i].take().unwrap();
        match node.prev {
            Some(p) => self.node_mut(p).next = node.next,
            None => self.head = node.next,
        }
        match node.next {
            Some(n) => self.node_mut(n).prev = node.prev,
            None => self.tail = node.prev,
        }
        self.free.push(i);
        self.count -= 1;
        node.value
    }

    pub fn push_front(&mut self, value: &str) {
        let i = self.create(value, None, self.head);
        match self.head {
            Some(h) => self.node_mut(h).prev = Some(i),
            None => self.tail = Some(i),
        }
        self.head = Some(i);
    }

    pub fn push_back(&mut self, value: &str) {
        let i = self.create(value, self.tail, None);
        match self.tail {
            Some(t) => self.node_mut(t).next = Some(i),
            None => self.head = Some(i),
        }
        self.tail = Some(i);
    }

    pub fn insert_after(&mut self, key: &str, value: &str) -> bool {
        let Some(current) = self.find(key) else {
            return false;
        };
        if self.tail == Some(current) {
            self.push_back(value);
            return true;
        }
        let next = self.node(current).next.unwrap();
        let i = self.create(value, Some(current), Some(next));
        self.node_mut(next).prev = Some(i);
        self.node_mut(current).next = Some(i);
        true
    }

    pub fn insert_before(&mut self, key: &str, value: &str) -> bool {
        let Some(current) = self.find(key) else {
            return false;
        };
        if self.head == Some(current) {
            self.push_front(value);
            return true;
        }
        let prev = self.node(current).prev.unwrap();
        let i = self.create(value, Some(prev), Some(current));
        self.node_mut(prev).next = Some(i);
        self.node_mut(current).prev = Some(i);
        true
    }

    pub fn pop_front(&mut self) -> Option<String> {
        let head = self.head?;
        Some(self.unlink(head))
    }

    pub fn pop_back(&mut self) -> Option<String> {
        let tail = self.tail?;
        Some(self.unlink(tail))
    }

    pub fn remove(&mut self, key: &str) -> bool {
        match self.find(key) {
            Some(i) => {
                self.unlink(i);
                true
            }
            None => false,
        }
    }

    pub fn get(&self, index: usize) -> Option<String> {
        if index >= self.count {
            return None;
        }
        let mut current = self.head.unwrap();
        for _ in 0..index {
            current = self.node(current).next.unwrap();
        }
        Some(self.node(current).value.clone())
    }

    pub fn contains(&self, key: &str) -> bool {
        self.find(key).is_some()
    }

    pub fn values(&self) -> Vec<String> {
        let mut result = Vec::new();
        let mut current = self.head;
        while let Some(i) = current {
            result.push(self.node(i).value.clone());
            current = self.node(i).next;
        }
        result
    }
}

impl Default for DoublyList {
    fn default() -> Self {
        Self::new()
    }
}

// ---------- ЛР №3: методы иерархии Container / Sequence ----------

impl Container for DoublyList {
    fn length(&self) -> usize {
        self.count
    }
    fn clear(&mut self) {
        *self = Self::new();
    }
    fn type_name(&self) -> &'static str {
        "DoublyList"
    }
}

impl Sequence for DoublyList {
    fn values(&self) -> Vec<String> {
        DoublyList::values(self)
    }
    fn restore(&mut self, value: &str) {
        self.push_back(value);
    }
}

impl_sequence_serializable!(DoublyList);
