use lab3::{
    Array, ChainHashTable, Container, DoublyList, Format, HashTable, OpenHashTable, Probing, Queue,
    RBTree, Sequence, SinglyList, Stack,
};
use std::path::Path;

fn dump_seq(s: &dyn Sequence) -> String {
    format!("[{}]", s.values().join(", "))
}

fn dump_table(t: &dyn HashTable) -> String {
    let items: Vec<String> = t
        .pairs()
        .into_iter()
        .map(|(k, v)| format!("{k}: {v}"))
        .collect();
    format!("{{{}}}", items.join(", "))
}

fn check<T: Container>(item: &T, empty: impl Fn() -> T, dir: &Path, show: impl Fn(&T) -> String) {
    println!("{} ({}): {}", item.type_name(), item.length(), show(item));
    for (format, ext) in [(Format::Binary, "bin"), (Format::Text, "json")] {
        let path = dir.join(format!("{}.{ext}", item.type_name().replace('/', "_")));
        let path = path.to_string_lossy().to_string();
        item.save_file(&path, format).expect("save");
        let mut loaded = empty();
        loaded.load_file(&path, format).expect("load");
        let ok = if show(&loaded) == show(item) {
            "OK"
        } else {
            "MISMATCH"
        };
        println!("  {ext} -> {path} -> {}  {ok}", show(&loaded));
    }
}

fn main() {
    let dir = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "data".to_string());
    let dir = Path::new(&dir);
    std::fs::create_dir_all(dir).expect("create data dir");

    let mut array = Array::new();
    for v in ["10", "20", "30"] {
        array.push_back(v);
    }
    array.insert(1, "15");
    check(&array, Array::new, dir, |a| dump_seq(a));

    let mut sl = SinglyList::new();
    sl.push_back("b");
    sl.push_back("c");
    sl.push_front("a");
    check(&sl, SinglyList::new, dir, |a| dump_seq(a));

    let mut dl = DoublyList::new();
    dl.push_back("x");
    dl.push_back("z");
    dl.insert_before("z", "y");
    check(&dl, DoublyList::new, dir, |a| dump_seq(a));

    let mut st = Stack::new();
    for v in ["1", "2", "3"] {
        st.push(v);
    }
    check(&st, Stack::new, dir, |a| dump_seq(a));

    let mut q = Queue::new();
    for v in ["first", "second", "third"] {
        q.push(v);
    }
    check(&q, Queue::new, dir, |a| dump_seq(a));

    let mut tree = RBTree::new();
    for v in ["10", "20", "30", "15", "5", "25"] {
        tree.insert(v);
    }
    check(&tree, RBTree::new, dir, |t| t.preorder().join(" "));

    let mut chain = ChainHashTable::new();
    let mut lin = OpenHashTable::new(Probing::Linear, 11);
    let mut dbl = OpenHashTable::new(Probing::Double, 11);
    for (k, v) in [("apple", "1"), ("pear", "2"), ("plum", "3")] {
        chain.put(k, v);
        lin.put(k, v);
        dbl.put(k, v);
    }
    check(&chain, ChainHashTable::new, dir, |t| dump_table(t));
    check(
        &lin,
        || OpenHashTable::new(Probing::Linear, 11),
        dir,
        |t| dump_table(t),
    );
    check(
        &dbl,
        || OpenHashTable::new(Probing::Double, 11),
        dir,
        |t| dump_table(t),
    );

    println!(
        "\nRBTree:\n{}valid: {}",
        tree.diagram(),
        if tree.is_valid() { "yes" } else { "no" }
    );
}
