use crate::container::{impl_sequence_serializable, Container, Sequence};
use crate::singly_list::SinglyList;

// Очередь на односвязном списке: добавление в хвост, удаление из головы
pub struct Queue {
    list: SinglyList,
}

impl Queue {
    pub fn new() -> Queue {
        Queue {
            list: SinglyList::new(),
        }
    }

    pub fn push(&mut self, value: &str) {
        self.list.push_back(value);
    }

    pub fn pop(&mut self) -> Option<String> {
        self.list.pop_front()
    }

    pub fn front(&self) -> Option<String> {
        self.list.front()
    }

    // От первого к последнему
    pub fn values(&self) -> Vec<String> {
        self.list.values()
    }
}

impl Default for Queue {
    fn default() -> Self {
        Self::new()
    }
}

// ---------- ЛР №3: методы иерархии Container / Sequence ----------

impl Container for Queue {
    fn length(&self) -> usize {
        self.list.length()
    }
    fn clear(&mut self) {
        self.list.clear();
    }
    fn type_name(&self) -> &'static str {
        "Queue"
    }
}

impl Sequence for Queue {
    fn values(&self) -> Vec<String> {
        Queue::values(self)
    }
    fn restore(&mut self, value: &str) {
        self.push(value);
    }
}

impl_sequence_serializable!(Queue);
