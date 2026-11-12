use crate::container::{impl_sequence_serializable, Container, Sequence};

// Самописный динамический массив строк (без Vec).
// Любое значение хранится как строка, поэтому в массив можно
// положить и числа (10, -5), и слова (a, b, abc).
pub struct Array {
    data: Box<[String]>, // буфер; его длина - это ёмкость
    size: usize,         // сколько элементов занято
}

// Буфер из n пустых строк
fn new_buffer(n: usize) -> Box<[String]> {
    (0..n).map(|_| String::new()).collect()
}

impl Array {
    pub fn new() -> Array {
        Array {
            data: new_buffer(4),
            size: 0,
        }
    }

    // Увеличить буфер в два раза и перенести элементы
    fn resize(&mut self) {
        let mut new_data = new_buffer(self.data.len() * 2);
        for i in 0..self.size {
            new_data[i] = std::mem::take(&mut self.data[i]);
        }
        self.data = new_data;
    }

    // Добавить в конец
    pub fn push_back(&mut self, value: &str) {
        if self.size == self.data.len() {
            self.resize();
        }
        self.data[self.size] = value.to_string();
        self.size += 1;
    }

    // Вставить по индексу (элементы справа сдвигаются вправо)
    pub fn insert(&mut self, index: usize, value: &str) -> bool {
        if index > self.size {
            return false;
        }
        if self.size == self.data.len() {
            self.resize();
        }
        let mut i = self.size;
        while i > index {
            self.data[i] = std::mem::take(&mut self.data[i - 1]);
            i -= 1;
        }
        self.data[index] = value.to_string();
        self.size += 1;
        true
    }

    // Прочитать по индексу
    pub fn get(&self, index: usize) -> Option<String> {
        if index >= self.size {
            return None;
        }
        Some(self.data[index].clone())
    }

    // Заменить по индексу
    pub fn set(&mut self, index: usize, value: &str) -> bool {
        if index >= self.size {
            return false;
        }
        self.data[index] = value.to_string();
        true
    }

    // Удалить по индексу (элементы справа сдвигаются влево)
    pub fn erase(&mut self, index: usize) -> Option<String> {
        if index >= self.size {
            return None;
        }
        let removed = std::mem::take(&mut self.data[index]);
        for i in index..self.size - 1 {
            self.data[i] = std::mem::take(&mut self.data[i + 1]);
        }
        self.size -= 1;
        Some(removed)
    }

    pub fn length(&self) -> usize {
        self.size
    }

    // Занятая часть буфера (для вывода и записи в файл)
    pub fn as_slice(&self) -> &[String] {
        &self.data[..self.size]
    }
}

impl Default for Array {
    fn default() -> Self {
        Self::new()
    }
}

// ---------- ЛР №3: методы иерархии Container / Sequence ----------

impl Container for Array {
    fn length(&self) -> usize {
        self.size
    }
    fn clear(&mut self) {
        self.size = 0;
    }
    fn type_name(&self) -> &'static str {
        "Array"
    }
}

impl Sequence for Array {
    fn values(&self) -> Vec<String> {
        self.as_slice().to_vec()
    }
    fn restore(&mut self, value: &str) {
        self.push_back(value);
    }
}

impl_sequence_serializable!(Array);
