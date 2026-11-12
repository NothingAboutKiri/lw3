use lab3::{Array, Container, DoublyList, Queue, RBTree, Sequence, SinglyList, Stack};

fn join(s: &dyn Sequence) -> String {
    s.values().join(" ")
}

#[test]
fn array_lab1_api() {
    let mut a = Array::default();
    assert!(a.is_empty());
    for v in ["10", "20", "30", "40", "50"] {
        a.push_back(v);
    }
    assert_eq!(a.length(), 5);
    assert_eq!(a.get(0).as_deref(), Some("10"));
    assert_eq!(a.get(5), None);
    assert_eq!(a.type_name(), "Array");
    let mut b = Array::new();
    assert!(b.insert(0, "b") && b.insert(0, "a") && b.insert(2, "d") && b.insert(2, "c"));
    assert!(!b.insert(5, "x"));
    assert_eq!(b.as_slice().join(" "), "a b c d");
    assert!(b.set(0, "one") && !b.set(4, "z"));
    assert_eq!(b.erase(1).as_deref(), Some("b"));
    assert_eq!(b.erase(3), None);
    assert_eq!(join(&b), "one c d");
    for i in 0..1000 {
        b.push_back(&i.to_string());
    }
    assert_eq!(b.get(1002).as_deref(), Some("999"));
    b.clear();
    assert!(b.is_empty());
    b.restore("again");
    assert_eq!(join(&b), "again");
}

#[test]
fn singly_list_lab1_api() {
    let mut l = SinglyList::default();
    assert_eq!(l.front(), None);
    assert!(!l.insert_before("x", "y") && !l.insert_after("x", "y") && !l.remove("x"));
    assert_eq!((l.pop_front(), l.pop_back()), (None, None));
    l.push_back("a");
    l.push_back("c");
    assert!(l.insert_after("a", "b") && l.insert_after("c", "d"));
    assert!(l.insert_before("a", "start") && l.insert_before("d", "c2"));
    assert!(!l.insert_after("missing", "x") && !l.insert_before("missing", "x"));
    l.push_back("tail");
    assert_eq!(join(&l), "start a b c c2 d tail");
    assert_eq!(l.front().as_deref(), Some("start"));
    assert_eq!(l.pop_back().as_deref(), Some("tail"));
    assert_eq!(l.pop_front().as_deref(), Some("start"));
    assert!(l.remove("a") && l.remove("d") && l.remove("c") && !l.remove("zzz"));
    assert!(l.contains("b") && !l.contains("a"));
    assert_eq!(l.get(1).as_deref(), Some("c2"));
    assert_eq!(l.get(2), None);
    l.push_back("e");
    assert_eq!(join(&l), "b c2 e");
    assert_eq!((l.length(), l.type_name()), (3, "SinglyList"));
    l.clear();
    assert!(l.is_empty());
    l.push_front("z");
    l.push_front("y");
    assert_eq!(l.pop_back().as_deref(), Some("z"));
    assert_eq!(l.pop_back().as_deref(), Some("y"));
    assert!(l.is_empty());
    l.restore("r");
    assert_eq!(join(&l), "r");
}

#[test]
fn doubly_list_lab1_api() {
    let mut l = DoublyList::default();
    assert!(!l.insert_after("x", "y") && !l.insert_before("x", "y") && !l.remove("x"));
    assert_eq!((l.pop_front(), l.pop_back()), (None, None));
    l.push_back("b");
    l.push_front("a0");
    assert!(l.remove("a0"));
    assert!(l.insert_before("b", "a") && l.insert_after("b", "d") && l.insert_after("b", "c"));
    assert!(l.insert_before("d", "c2") && l.insert_after("d", "e"));
    assert_eq!(join(&l), "a b c c2 d e");
    assert_eq!(l.pop_back().as_deref(), Some("e"));
    assert_eq!(l.pop_front().as_deref(), Some("a"));
    assert!(l.contains("c") && !l.contains("zz"));
    assert_eq!(l.get(1).as_deref(), Some("c"));
    assert_eq!(l.get(9), None);
    assert!(l.remove("c") && !l.remove("c"));
    assert_eq!((l.length(), l.type_name()), (3, "DoublyList"));
    l.clear();
    assert!(l.is_empty());
    l.restore("x");
    l.push_front("w");
    assert_eq!(join(&l), "w x");
}

#[test]
fn stack_and_queue_lab1_api() {
    let mut s = Stack::default();
    assert_eq!((s.pop(), s.top()), (None, None));
    for v in ["1", "2", "3"] {
        s.push(v);
    }
    assert_eq!(s.top().as_deref(), Some("3"));
    assert_eq!(join(&s), "3 2 1");
    assert_eq!(s.pop().as_deref(), Some("3"));
    assert_eq!((s.length(), s.type_name()), (2, "Stack"));
    s.clear();
    assert!(s.is_empty());
    s.restore("top");
    s.restore("bottom");
    assert_eq!(s.pop().as_deref(), Some("top"));

    let mut q = Queue::default();
    assert_eq!((q.pop(), q.front()), (None, None));
    for v in ["a", "b", "c"] {
        q.push(v);
    }
    assert_eq!(q.front().as_deref(), Some("a"));
    assert_eq!(q.pop().as_deref(), Some("a"));
    q.push("d");
    assert_eq!(join(&q), "b c d");
    assert_eq!((q.length(), q.type_name()), (3, "Queue"));
    q.clear();
    assert!(q.is_empty());
    q.restore("x");
    assert_eq!(q.pop().as_deref(), Some("x"));
}

#[test]
fn tree_values_are_sorted_keys() {
    let mut t = RBTree::new();
    for v in ["2", "10", "-5", "abc", "99"] {
        t.insert(v);
    }
    assert_eq!(join(&t), "-5 2 10 99 abc");
    assert_eq!(t.keys(), t.values());
}
