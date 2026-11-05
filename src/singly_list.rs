use crate::container::{impl_sequence_serializable, Container, Sequence};

// Односвязный список. Узлы хранятся в векторе nodes,
// вместо указателей используются номера узлов (Option<usize>).
struct Node {
    value: String,
    next: Option<usize>,
}

pub struct SinglyList {
    nodes: Vec<Option<Node>>, // None - свободное место после удаления
    free: Vec<usize>,         // номера свободных мест
    head: Option<usize>,
    tail: Option<usize>,
    count: usize,
}

impl SinglyList {
    pub fn new() -> SinglyList {
        SinglyList {
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

    // Создать узел и вернуть его номер
    fn create(&mut self, value: &str, next: Option<usize>) -> usize {
        let node = Node {
            value: value.to_string(),
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

    // Удалить узел и вернуть его значение
    fn release(&mut self, i: usize) -> String {
        let node = self.nodes[i].take().unwrap();
        self.free.push(i);
        self.count -= 1;
        node.value
    }

    pub fn push_front(&mut self, value: &str) {
        let i = self.create(value, self.head);
        self.head = Some(i);
        if self.tail.is_none() {
            self.tail = Some(i);
        }
    }

    pub fn push_back(&mut self, value: &str) {
        let i = self.create(value, None);
        match self.tail {
            Some(t) => self.node_mut(t).next = Some(i),
            None => self.head = Some(i),
        }
        self.tail = Some(i);
    }

    pub fn insert_after(&mut self, key: &str, value: &str) -> bool {
        let mut current = self.head;
        while let Some(i) = current {
            if self.node(i).value == key {
                let next = self.node(i).next;
                let new = self.create(value, next);
                self.node_mut(i).next = Some(new);
                if self.tail == Some(i) {
                    self.tail = Some(new);
                }
                return true;
            }
            current = self.node(i).next;
        }
        false
    }

    pub fn insert_before(&mut self, key: &str, value: &str) -> bool {
        let Some(head) = self.head else {
            return false;
        };
        if self.node(head).value == key {
            self.push_front(value);
            return true;
        }
        let mut previous = head; // ищем узел, стоящий перед key
        while let Some(next) = self.node(previous).next {
            if self.node(next).value == key {
                let new = self.create(value, Some(next));
                self.node_mut(previous).next = Some(new);
                return true;
            }
            previous = next;
        }
        false
    }

    pub fn pop_front(&mut self) -> Option<String> {
        let head = self.head?;
        self.head = self.node(head).next;
        if self.head.is_none() {
            self.tail = None;
        }
        Some(self.release(head))
    }

    pub fn pop_back(&mut self) -> Option<String> {
        let tail = self.tail?;
        if self.head == Some(tail) {
            return self.pop_front();
        }
        let mut previous = self.head.unwrap(); // ищем предпоследний узел
        while self.node(previous).next != Some(tail) {
            previous = self.node(previous).next.unwrap();
        }
        self.node_mut(previous).next = None;
        self.tail = Some(previous);
        Some(self.release(tail))
    }

    pub fn remove(&mut self, key: &str) -> bool {
        let Some(head) = self.head else {
            return false;
        };
        if self.node(head).value == key {
            self.pop_front();
            return true;
        }
        let mut previous = head;
        while let Some(next) = self.node(previous).next {
            if self.node(next).value == key {
                let after = self.node(next).next;
                self.node_mut(previous).next = after;
                if self.tail == Some(next) {
                    self.tail = Some(previous);
                }
                self.release(next);
                return true;
            }
            previous = next;
        }
        false
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
        let mut current = self.head;
        while let Some(i) = current {
            if self.node(i).value == key {
                return true;
            }
            current = self.node(i).next;
        }
        false
    }

    pub fn front(&self) -> Option<String> {
        let head = self.head?;
        Some(self.node(head).value.clone())
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

impl Default for SinglyList {
    fn default() -> Self {
        Self::new()
    }
}

// ---------- ЛР №3: методы иерархии Container / Sequence ----------

impl Container for SinglyList {
    fn length(&self) -> usize {
        self.count
    }
    fn clear(&mut self) {
        *self = Self::new();
    }
    fn type_name(&self) -> &'static str {
        "SinglyList"
    }
}

impl Sequence for SinglyList {
    fn values(&self) -> Vec<String> {
        SinglyList::values(self)
    }
    fn restore(&mut self, value: &str) {
        self.push_back(value);
    }
}

impl_sequence_serializable!(SinglyList);
