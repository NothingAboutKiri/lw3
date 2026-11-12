use crate::array::Array;
use crate::container::LoadError;
use std::collections::BTreeMap;
use std::fmt::Write as _;
use std::io::{self, BufRead, Read, Write};

const MAGIC: &[u8; 4] = b"L3B1";
const MAX_STRING: u32 = 1 << 26;

pub fn write_u32(out: &mut dyn Write, value: u32) -> io::Result<()> {
    out.write_all(&value.to_le_bytes())
}

pub fn read_u32(input: &mut dyn Read) -> Result<u32, LoadError> {
    let mut bytes = [0u8; 4];
    input.read_exact(&mut bytes)?;
    Ok(u32::from_le_bytes(bytes))
}

pub fn write_string(out: &mut dyn Write, value: &str) -> io::Result<()> {
    write_u32(
        out,
        u32::try_from(value.len()).map_err(|_| io::Error::other("string too long"))?,
    )?;
    out.write_all(value.as_bytes())
}

pub fn read_string(input: &mut dyn Read) -> Result<String, LoadError> {
    let len = read_u32(input)?;
    if len > MAX_STRING {
        return Err(LoadError::Format("string is too long".into()));
    }
    let mut bytes = vec![0u8; len as usize];
    input.read_exact(&mut bytes)?;
    String::from_utf8(bytes).map_err(|_| LoadError::Format("string is not UTF-8".into()))
}

pub fn write_binary_header(out: &mut dyn Write, type_name: &str, count: usize) -> io::Result<()> {
    out.write_all(MAGIC)?;
    write_string(out, type_name)?;
    write_u32(
        out,
        u32::try_from(count).map_err(|_| io::Error::other("too many elements"))?,
    )
}

pub fn read_binary_header(input: &mut dyn Read, type_name: &str) -> Result<u32, LoadError> {
    let mut magic = [0u8; 4];
    input.read_exact(&mut magic)?;
    if &magic != MAGIC {
        return Err(LoadError::Format("not a lab3 binary file".into()));
    }
    if read_string(input)? != type_name {
        return Err(LoadError::Format(format!(
            "file contains another type, expected {type_name}"
        )));
    }
    read_u32(input)
}

// ---------- Текстовый формат: JSON из ЛР №1 (file.rs) ----------
// Файл вида { "Тип": ["x", "y", ...] }

// Строка JSON в кавычках: " и \ экранируются, управляющие символы - через \u00XX
fn json_string(text: &str) -> String {
    let mut result = String::from("\"");
    for c in text.chars() {
        if c == '"' || c == '\\' {
            result.push('\\');
            result.push(c);
        } else if (c as u32) < 0x20 {
            let _ = write!(result, "\\u{:04x}", c as u32);
        } else {
            result.push(c);
        }
    }
    result.push('"');
    result
}

// Одна запись файла: "имя": ["x", "y", ...]
fn write_item(text: &mut String, first: &mut bool, name: &str, values: &[String]) {
    if !*first {
        text.push_str(",\n");
    }
    *first = false;
    let _ = write!(text, "  {}: [", json_string(name));
    for (i, value) in values.iter().enumerate() {
        if i > 0 {
            text.push_str(", ");
        }
        text.push_str(&json_string(value));
    }
    text.push(']');
}

// Простой разбор JSON: позиция pos движется по символам текста
struct Parser {
    chars: Vec<char>,
    pos: usize,
}

impl Parser {
    // Пропустить пробелы, табуляции и переводы строк
    fn skip_spaces(&mut self) {
        while self.pos < self.chars.len()
            && matches!(self.chars[self.pos], ' ' | '\t' | '\n' | '\r')
        {
            self.pos += 1;
        }
    }

    // Ожидается символ c (после пробелов)
    fn expect(&mut self, c: char) -> bool {
        self.skip_spaces();
        if self.pos < self.chars.len() && self.chars[self.pos] == c {
            self.pos += 1;
            return true;
        }
        false
    }

    fn next(&mut self) -> Option<char> {
        let c = *self.chars.get(self.pos)?;
        self.pos += 1;
        Some(c)
    }

    // Строка в кавычках с escape-последовательностями
    fn read_string(&mut self) -> Option<String> {
        if !self.expect('"') {
            return None;
        }
        let mut out = String::new();
        loop {
            let c = self.next()?;
            if c == '"' {
                return Some(out);
            }
            if c != '\\' {
                out.push(c);
                continue;
            }
            match self.next()? {
                '"' => out.push('"'),
                '\\' => out.push('\\'),
                '/' => out.push('/'),
                'n' => out.push('\n'),
                't' => out.push('\t'),
                'r' => out.push('\r'),
                'b' => out.push('\u{8}'),
                'f' => out.push('\u{c}'),
                'u' => {
                    let mut code = 0u32;
                    for _ in 0..4 {
                        code = code * 16 + self.next()?.to_digit(16)?;
                    }
                    out.push(char::from_u32(code)?);
                }
                _ => return None,
            }
        }
    }

    // Значение в массиве: строка в кавычках или число без кавычек (например, 15)
    fn read_value(&mut self) -> Option<String> {
        self.skip_spaces();
        if self.pos < self.chars.len() && self.chars[self.pos] == '"' {
            return self.read_string();
        }
        let mut out = String::new();
        while self.pos < self.chars.len()
            && !matches!(self.chars[self.pos], ',' | ']' | ' ' | '\n' | '\r' | '\t')
        {
            out.push(self.chars[self.pos]);
            self.pos += 1;
        }
        if out.is_empty() {
            None
        } else {
            Some(out)
        }
    }

    // Весь файл: { "имя": [значения], ... }
    fn parse(&mut self) -> Option<BTreeMap<String, Array>> {
        let mut result = BTreeMap::new();
        if !self.expect('{') {
            return None;
        }
        if !self.expect('}') {
            loop {
                let name = self.read_string()?;
                if !self.expect(':') || !self.expect('[') {
                    return None;
                }
                let mut values = Array::new();
                if !self.expect(']') {
                    loop {
                        let value = self.read_value()?;
                        values.push_back(&value);
                        if self.expect(']') {
                            break;
                        }
                        if !self.expect(',') {
                            return None;
                        }
                    }
                }
                result.insert(name, values);
                if self.expect('}') {
                    break;
                }
                if !self.expect(',') {
                    return None;
                }
            }
        }
        self.skip_spaces();
        if self.pos != self.chars.len() {
            return None;
        }
        Some(result)
    }
}

pub fn write_json(out: &mut dyn Write, type_name: &str, values: &[String]) -> io::Result<()> {
    let mut text = String::from("{\n");
    let mut first = true;
    write_item(&mut text, &mut first, type_name, values);
    text.push_str("\n}\n");
    out.write_all(text.as_bytes())
}

pub fn read_json(input: &mut dyn BufRead, type_name: &str) -> Result<Vec<String>, LoadError> {
    let mut text = String::new();
    input
        .read_to_string(&mut text)
        .map_err(|_| LoadError::Format("text file is not UTF-8".into()))?;
    let mut parser = Parser {
        chars: text.chars().collect(),
        pos: 0,
    };
    let result = parser
        .parse()
        .ok_or_else(|| LoadError::Format("not a lab3 text (JSON) file".into()))?;
    match result.get(type_name) {
        Some(values) if result.len() == 1 => Ok(values.as_slice().to_vec()),
        _ => Err(LoadError::Format(format!(
            "file contains another type, expected {type_name}"
        ))),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn u32_and_string_roundtrip() {
        let mut buf = Vec::new();
        write_u32(&mut buf, 0x0102_0304).unwrap();
        write_string(&mut buf, "привет").unwrap();
        let mut r = buf.as_slice();
        assert_eq!(read_u32(&mut r).unwrap(), 0x0102_0304);
        assert_eq!(read_string(&mut r).unwrap(), "привет");
        assert!(read_u32(&mut r).is_err());
    }

    #[test]
    fn string_limits_and_utf8() {
        let mut huge = Vec::new();
        write_u32(&mut huge, MAX_STRING + 1).unwrap();
        assert!(matches!(
            read_string(&mut huge.as_slice()),
            Err(LoadError::Format(_))
        ));
        let mut bad = Vec::new();
        write_u32(&mut bad, 2).unwrap();
        bad.extend_from_slice(&[0xff, 0xfe]);
        assert!(read_string(&mut bad.as_slice()).is_err());
    }

    #[test]
    fn binary_headers() {
        let mut buf = Vec::new();
        write_binary_header(&mut buf, "Array", 3).unwrap();
        assert_eq!(read_binary_header(&mut buf.as_slice(), "Array").unwrap(), 3);
        assert!(read_binary_header(&mut buf.as_slice(), "Queue").is_err());
        assert!(read_binary_header(&mut b"XXXXrest".as_slice(), "Array").is_err());
    }

    fn json(text: &str) -> Result<Vec<String>, LoadError> {
        read_json(&mut text.as_bytes(), "Array")
    }

    #[test]
    fn json_write_and_read() {
        let values: Vec<String> = ["a\"b", "c\\d", "e\nf", "\u{1}", "ж"]
            .iter()
            .map(ToString::to_string)
            .collect();
        let mut out = Vec::new();
        write_json(&mut out, "Array", &values).unwrap();
        let text = String::from_utf8(out).unwrap();
        assert_eq!(
            text,
            "{\n  \"Array\": [\"a\\\"b\", \"c\\\\d\", \"e\\u000af\", \"\\u0001\", \"ж\"]\n}\n"
        );
        assert_eq!(json(&text).unwrap(), values);
        assert_eq!(
            json(" { \"Array\" :\n [ 15 ,\t\"x\" ] } ").unwrap(),
            ["15", "x"]
        );
        assert_eq!(json("{\"Array\": []}").unwrap(), Vec::<String>::new());
        assert_eq!(
            json("{\"Array\": [\"\\t\\r\\/\\b\\f\\n\\u0041\"]}").unwrap(),
            ["\t\r/\u{8}\u{c}\nA"]
        );
    }

    #[test]
    fn json_errors() {
        for bad in [
            "",
            "[]",
            "{}",
            "{\"Array\": [\"1\"",
            "{\"Array\": [\"1\" \"2\"]}",
            "{\"Array\" [\"1\"]}",
            "{\"Array\": [\"bad\\q\"]}",
            "{\"Array\": [\"\\u12\"]}",
            "{\"Array\": [\"\\ud800\"]}",
            "{\"Array\": [\"open",
            "{\"Queue\": [\"x\"]}",
            "{\"Array\": [], \"Queue\": []}",
            "{\"Array\": []} tail",
            "{\"Array\": [,]}",
            "{\"Array\": [\"\\",
        ] {
            assert!(json(bad).is_err(), "{bad}");
        }
        assert!(read_json(&mut [0xffu8, 0xfe].as_slice(), "Array").is_err());
    }

    #[test]
    fn load_error_display() {
        let io_err: LoadError = io::Error::other("boom").into();
        assert!(io_err.to_string().contains("io error"));
        let f = LoadError::Format("x".into());
        assert_eq!(f.to_string(), "bad data: x");
        let eof: LoadError = io::Error::from(io::ErrorKind::UnexpectedEof).into();
        assert!(matches!(eof, LoadError::Format(_)));
    }
}
