use lab3::{
    Array, ChainHashTable, Container, DoublyList, Format, HashTable, LoadError, OpenHashTable,
    Probing, Queue, RBTree, Sequence, Serializable, SinglyList, Stack,
};

fn roundtrip<T: Serializable>(item: &T, mut empty: T, format: Format) -> T {
    let mut buf = Vec::new();
    item.save(&mut buf, format).unwrap();
    empty.load(&mut buf.as_slice(), format).unwrap();
    empty
}

fn text_of(item: &dyn Serializable) -> String {
    let mut buf = Vec::new();
    item.save_text(&mut buf).unwrap();
    String::from_utf8(buf).unwrap()
}

fn load_text<T: Serializable>(item: &mut T, text: &str) -> Result<(), LoadError> {
    item.load(&mut text.as_bytes(), Format::Text)
}

// Одна проверка для любого контейнера: сохранить в обоих форматах, загрузить в пустой и сравнить
fn check_roundtrip<T: Container>(
    filled: &T,
    make_empty: impl Fn() -> T,
    show: impl Fn(&T) -> String,
) {
    for format in [Format::Binary, Format::Text] {
        let mut buf = Vec::new();
        filled.save(&mut buf, format).unwrap();
        let mut loaded = make_empty();
        loaded.load(&mut buf.as_slice(), format).unwrap();
        assert_eq!(
            loaded.length(),
            filled.length(),
            "{} {format:?}",
            filled.type_name()
        );
        assert_eq!(
            show(&loaded),
            show(filled),
            "{} {format:?}",
            filled.type_name()
        );
    }
}

fn show_seq<T: Sequence>(s: &T) -> String {
    s.values().join(" ")
}

fn show_table<T: HashTable>(t: &T) -> String {
    format!("{:?}", t.pairs())
}

fn fill_table<T: HashTable>(mut t: T) -> T {
    t.put("k1", "v1");
    t.put("key with\ttab", "multi\nline");
    t
}

#[test]
fn roundtrip_all_containers() {
    let mut a = Array::new();
    for v in [
        "10",
        "with space",
        "tab\there",
        "line\nbreak",
        "back\\slash",
        "quote\"",
        "",
    ] {
        a.push_back(v);
    }
    check_roundtrip(&a, Array::new, show_seq);

    let mut sl = SinglyList::new();
    sl.push_back("a");
    sl.push_back("b");
    check_roundtrip(&sl, SinglyList::new, show_seq);

    let mut dl = DoublyList::new();
    for v in ["x", "y", "z"] {
        dl.push_back(v);
    }
    check_roundtrip(&dl, DoublyList::new, show_seq);

    let mut st = Stack::new();
    for v in ["1", "2", "3"] {
        st.push(v);
    }
    check_roundtrip(&st, Stack::new, show_seq);

    let mut q = Queue::new();
    q.push("first");
    q.push("second");
    check_roundtrip(&q, Queue::new, show_seq);

    let mut t = RBTree::new();
    for v in ["10", "20", "30", "15", "5", "25"] {
        t.insert(v);
    }
    check_roundtrip(&t, RBTree::new, |t| t.preorder().join(" "));

    check_roundtrip(
        &fill_table(ChainHashTable::new()),
        ChainHashTable::new,
        show_table,
    );
    for probing in [Probing::Linear, Probing::Double] {
        check_roundtrip(
            &fill_table(OpenHashTable::new(probing, 11)),
            || OpenHashTable::new(probing, 11),
            show_table,
        );
    }
}

#[test]
fn text_format_is_lab1_json() {
    let mut sl = SinglyList::new();
    sl.push_back("a");
    sl.push_back("b");
    assert_eq!(text_of(&sl), "{\n  \"SinglyList\": [\"a\", \"b\"]\n}\n");
    let mut t = RBTree::new();
    for v in ["10", "20", "30", "15", "5", "25"] {
        t.insert(v);
    }
    assert_eq!(
        text_of(&t),
        "{\n  \"RBTree\": [\"20(B)\", \"10(B)\", \"5(R)\", \"15(R)\", \"30(B)\", \"25(R)\"]\n}\n"
    );
    let mut h = ChainHashTable::new();
    h.put("k", "v");
    assert_eq!(text_of(&h), "{\n  \"ChainHashTable\": [\"k\", \"v\"]\n}\n");
    let mut st = Stack::new();
    for v in ["1", "2", "3"] {
        st.push(v);
    }
    assert_eq!(
        roundtrip(&st, Stack::new(), Format::Text).pop().as_deref(),
        Some("3")
    );
}

#[test]
fn hash_table_bad_data() {
    let mut h = ChainHashTable::new();
    assert!(load_text(&mut h, "{\"ChainHashTable\": [\"key\"]}").is_err());
    let mut cut = Vec::new();
    fill_table(ChainHashTable::new())
        .save_binary(&mut cut)
        .unwrap();
    cut.pop();
    assert!(h.load(&mut cut.as_slice(), Format::Binary).is_err());
}

#[test]
fn files_roundtrip() {
    let dir = std::env::temp_dir();
    let bin = dir
        .join("lab3_rust_array.bin")
        .to_string_lossy()
        .to_string();
    let txt = dir
        .join("lab3_rust_array.json")
        .to_string_lossy()
        .to_string();
    let mut a = Array::new();
    for v in ["1", "2", "3"] {
        a.push_back(v);
    }
    a.save_file(&bin, Format::Binary).unwrap();
    a.save_file(&txt, Format::Text).unwrap();
    let mut b = Array::new();
    b.load_file(&bin, Format::Binary).unwrap();
    assert_eq!(b.values(), ["1", "2", "3"]);
    let mut c = Array::new();
    c.load_file(&txt, Format::Text).unwrap();
    assert_eq!(c.values(), ["1", "2", "3"]);
    std::fs::remove_file(bin).unwrap();
    std::fs::remove_file(txt).unwrap();
    assert!(matches!(
        c.load_file("no/such/dir/file.bin", Format::Binary),
        Err(LoadError::Io(_))
    ));
    assert!(c.save_file("no/such/dir/file.bin", Format::Binary).is_err());
}

#[test]
fn rejects_bad_data_and_keeps_old_content() {
    let mut a = Array::new();
    a.push_back("x");
    let mut bin = Vec::new();
    a.save_binary(&mut bin).unwrap();
    assert!(SinglyList::new()
        .load(&mut bin.as_slice(), Format::Binary)
        .is_err());
    let mut cut = bin.clone();
    cut.pop();
    assert!(a.load(&mut cut.as_slice(), Format::Binary).is_err());
    assert_eq!(a.values(), ["x"]);
    assert!(load_text(&mut a, "{\"Queue\": [\"y\"]}").is_err());
    assert_eq!(a.values(), ["x"]);
    load_text(&mut a, "{\"Array\": [15, -3]}").unwrap();
    assert_eq!(a.values(), ["15", "-3"]);
}

#[test]
fn rejects_bad_tree() {
    let mut t = RBTree::new();
    load_text(&mut t, "{\"RBTree\": [\"5(B)\", \"3(R)\", \"8(R)\"]}").unwrap();
    for bad in [
        "{\"RBTree\": [\"5(X)\"]}",
        "{\"RBTree\": [\"5\"]}",
        "{\"RBTree\": [\"(B)\"]}",
        "{\"RBTree\": [\"5(R)\"]}",
        "{\"RBTree\": [\"5(B)\", \"5(R)\"]}",
        "{\"RBTree\": [\"5(B)\", \"3(B)\"]}",
    ] {
        assert!(load_text(&mut t, bad).is_err(), "{bad}");
        assert_eq!(t.preorder().join(" "), "5(B) 3(R) 8(R)");
    }
    let mut bin = Vec::new();
    t.save_binary(&mut bin).unwrap();
    bin.truncate(bin.len() - 2);
    assert!(t.load(&mut bin.as_slice(), Format::Binary).is_err());
    assert_eq!(t.length(), 3);
}

#[test]
fn polymorphic_save() {
    let mut all: Vec<Box<dyn Container>> = vec![
        Box::new(Array::new()),
        Box::new(SinglyList::new()),
        Box::new(DoublyList::new()),
        Box::new(Stack::new()),
        Box::new(Queue::new()),
        Box::new(RBTree::new()),
        Box::new(ChainHashTable::new()),
        Box::new(OpenHashTable::new(Probing::Double, 11)),
    ];
    for c in &mut all {
        assert!(c.is_empty());
        let mut out = Vec::new();
        c.save(&mut out, Format::Text).unwrap();
        assert_eq!(
            String::from_utf8(out.clone()).unwrap(),
            format!("{{\n  \"{}\": []\n}}\n", c.type_name())
        );
        c.load(&mut out.as_slice(), Format::Text).unwrap();
        assert_eq!(c.length(), 0);
    }
}
