use crate::container::{impl_sequence_serializable, Container, Sequence};
use crate::singly_list::SinglyList;

// Стек на односвязном списке: вершина - голова списка
pub struct Stack {
    list: SinglyList,
}

impl Stack {
    pub fn new() -> Stack {
        Stack {
            list: SinglyList::new(),
        }
    }

    pub fn push(&mut self, value: &str) {
        self.list.push_front(value);
    }

    pub fn pop(&mut self) -> Option<String> {
        self.list.pop_front()
    }

    pub fn top(&self) -> Option<String> {
        self.list.front()
    }

    // От вершины ко дну
    pub fn values(&self) -> Vec<String> {
        self.list.values()
    }
}

impl Default for Stack {
    fn default() -> Self {
        Self::new()
    }
}

// ---------- ЛР №3: методы иерархии Container / Sequence ----------

impl Container for Stack {
    fn length(&self) -> usize {
        self.list.length()
    }
    fn clear(&mut self) {
        self.list.clear();
    }
    fn type_name(&self) -> &'static str {
        "Stack"
    }
}

impl Sequence for Stack {
    fn values(&self) -> Vec<String> {
        Stack::values(self)
    }
    // значения сохраняются от вершины ко дну, поэтому каждое следующее кладётся под предыдущее
    fn restore(&mut self, value: &str) {
        self.list.push_back(value);
    }
}

impl_sequence_serializable!(Stack);
