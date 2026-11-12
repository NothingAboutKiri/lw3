use std::fmt;
use std::fs::File;
use std::io::{self, BufRead, BufReader, BufWriter, Read, Write};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Format {
    Binary,
    Text,
}

#[derive(Debug)]
pub enum LoadError {
    Io(io::Error),
    Format(String),
}

impl fmt::Display for LoadError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            LoadError::Io(e) => write!(f, "io error: {e}"),
            LoadError::Format(m) => write!(f, "bad data: {m}"),
        }
    }
}

impl std::error::Error for LoadError {}

impl From<io::Error> for LoadError {
    fn from(e: io::Error) -> Self {
        if e.kind() == io::ErrorKind::UnexpectedEof {
            LoadError::Format("unexpected end of data".to_string())
        } else {
            LoadError::Io(e)
        }
    }
}

pub trait Serializable {
    fn save_binary(&self, out: &mut dyn Write) -> io::Result<()>;
    fn load_binary(&mut self, input: &mut dyn Read) -> Result<(), LoadError>;
    fn save_text(&self, out: &mut dyn Write) -> io::Result<()>;
    fn load_text(&mut self, input: &mut dyn BufRead) -> Result<(), LoadError>;

    fn save(&self, out: &mut dyn Write, format: Format) -> io::Result<()> {
        match format {
            Format::Binary => self.save_binary(out),
            Format::Text => self.save_text(out),
        }
    }

    fn load(&mut self, input: &mut dyn BufRead, format: Format) -> Result<(), LoadError> {
        match format {
            Format::Binary => self.load_binary(input),
            Format::Text => self.load_text(input),
        }
    }

    fn save_file(&self, path: &str, format: Format) -> io::Result<()> {
        let mut out = BufWriter::new(File::create(path)?);
        self.save(&mut out, format)?;
        out.flush()
    }

    fn load_file(&mut self, path: &str, format: Format) -> Result<(), LoadError> {
        let mut input = BufReader::new(File::open(path)?);
        self.load(&mut input, format)
    }
}

pub trait Container: Serializable {
    fn length(&self) -> usize;
    fn is_empty(&self) -> bool {
        self.length() == 0
    }
    fn clear(&mut self);
    fn type_name(&self) -> &'static str;
}

// Последовательность значений: сериализация общая, через values() и restore()
pub trait Sequence: Container {
    fn values(&self) -> Vec<String>;
    fn restore(&mut self, value: &str);
}

// При ошибке чтения контейнер не меняется: сначала читаются все значения, потом заменяется содержимое
fn replace_all<S: Sequence + ?Sized>(s: &mut S, values: &[String]) {
    s.clear();
    for v in values {
        s.restore(v);
    }
}

pub(crate) fn save_sequence_binary<S: Sequence + ?Sized>(
    s: &S,
    out: &mut dyn Write,
) -> io::Result<()> {
    let values = s.values();
    crate::codec::write_binary_header(out, s.type_name(), values.len())?;
    for v in &values {
        crate::codec::write_string(out, v)?;
    }
    Ok(())
}

pub(crate) fn read_binary_values(
    input: &mut dyn Read,
    type_name: &str,
) -> Result<Vec<String>, LoadError> {
    let count = crate::codec::read_binary_header(input, type_name)?;
    (0..count)
        .map(|_| crate::codec::read_string(input))
        .collect()
}

pub(crate) fn load_sequence_binary<S: Sequence + ?Sized>(
    s: &mut S,
    input: &mut dyn Read,
) -> Result<(), LoadError> {
    let values = read_binary_values(input, s.type_name())?;
    replace_all(s, &values);
    Ok(())
}

pub(crate) fn save_sequence_text<S: Sequence + ?Sized>(
    s: &S,
    out: &mut dyn Write,
) -> io::Result<()> {
    crate::codec::write_json(out, s.type_name(), &s.values())
}

pub(crate) fn load_sequence_text<S: Sequence + ?Sized>(
    s: &mut S,
    input: &mut dyn BufRead,
) -> Result<(), LoadError> {
    let values = crate::codec::read_json(input, s.type_name())?;
    replace_all(s, &values);
    Ok(())
}

macro_rules! impl_sequence_serializable {
    ($t:ty) => {
        impl $crate::container::Serializable for $t {
            fn save_binary(&self, out: &mut dyn std::io::Write) -> std::io::Result<()> {
                $crate::container::save_sequence_binary(self, out)
            }
            fn load_binary(
                &mut self,
                input: &mut dyn std::io::Read,
            ) -> Result<(), $crate::container::LoadError> {
                $crate::container::load_sequence_binary(self, input)
            }
            fn save_text(&self, out: &mut dyn std::io::Write) -> std::io::Result<()> {
                $crate::container::save_sequence_text(self, out)
            }
            fn load_text(
                &mut self,
                input: &mut dyn std::io::BufRead,
            ) -> Result<(), $crate::container::LoadError> {
                $crate::container::load_sequence_text(self, input)
            }
        }
    };
}
pub(crate) use impl_sequence_serializable;
