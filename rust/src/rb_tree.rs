use crate::codec;
use crate::container::{read_binary_values, Container, LoadError, Sequence, Serializable};
use std::cmp::Ordering;
use std::collections::VecDeque;
use std::fmt::Write as _;
use std::io::{self, BufRead, Read, Write};

const NIL: usize = 0;

#[derive(Debug, Clone)]
struct Node {
    key: String,
    red: bool,
    left: usize,
    right: usize,
    parent: usize,
}

#[derive(Debug, Clone)]
pub struct RBTree {
    nodes: Vec<Node>,
    free: Vec<usize>,
    root: usize,
    count: usize,
}

fn is_number(s: &str) -> bool {
    let digits = s.strip_prefix('-').unwrap_or(s);
    (1..=18).contains(&digits.len()) && digits.bytes().all(|b| b.is_ascii_digit())
}

impl Default for RBTree {
    fn default() -> Self {
        Self::new()
    }
}

impl RBTree {
    pub fn new() -> Self {
        let nil = Node {
            key: String::new(),
            red: false,
            left: NIL,
            right: NIL,
            parent: NIL,
        };
        RBTree {
            nodes: vec![nil],
            free: Vec::new(),
            root: NIL,
            count: 0,
        }
    }

    pub fn compare(a: &str, b: &str) -> Ordering {
        match (is_number(a), is_number(b)) {
            (true, true) => {
                let x: i64 = a.parse().expect("number");
                let y: i64 = b.parse().expect("number");
                x.cmp(&y).then_with(|| a.cmp(b))
            }
            (true, false) => Ordering::Less,
            (false, true) => Ordering::Greater,
            (false, false) => a.cmp(b),
        }
    }

    pub fn less(a: &str, b: &str) -> bool {
        Self::compare(a, b) == Ordering::Less
    }

    fn alloc(&mut self, key: &str, red: bool, parent: usize) -> usize {
        let node = Node {
            key: key.to_string(),
            red,
            left: NIL,
            right: NIL,
            parent,
        };
        self.count += 1;
        if let Some(i) = self.free.pop() {
            self.nodes[i] = node;
            i
        } else {
            self.nodes.push(node);
            self.nodes.len() - 1
        }
    }

    fn search(&self, key: &str) -> usize {
        let mut x = self.root;
        while x != NIL {
            match Self::compare(key, &self.nodes[x].key) {
                Ordering::Equal => return x,
                Ordering::Less => x = self.nodes[x].left,
                Ordering::Greater => x = self.nodes[x].right,
            }
        }
        NIL
    }

    fn minimum(&self, mut x: usize) -> usize {
        while self.nodes[x].left != NIL {
            x = self.nodes[x].left;
        }
        x
    }

    fn maximum(&self, mut x: usize) -> usize {
        while self.nodes[x].right != NIL {
            x = self.nodes[x].right;
        }
        x
    }

    fn replace_child(&mut self, parent: usize, old: usize, new: usize) {
        if parent == NIL {
            self.root = new;
        } else if self.nodes[parent].left == old {
            self.nodes[parent].left = new;
        } else {
            self.nodes[parent].right = new;
        }
    }

    fn left_rotate(&mut self, x: usize) {
        let y = self.nodes[x].right;
        self.nodes[x].right = self.nodes[y].left;
        if self.nodes[y].left != NIL {
            let yl = self.nodes[y].left;
            self.nodes[yl].parent = x;
        }
        let xp = self.nodes[x].parent;
        self.nodes[y].parent = xp;
        self.replace_child(xp, x, y);
        self.nodes[y].left = x;
        self.nodes[x].parent = y;
    }

    fn right_rotate(&mut self, x: usize) {
        let y = self.nodes[x].left;
        self.nodes[x].left = self.nodes[y].right;
        if self.nodes[y].right != NIL {
            let yr = self.nodes[y].right;
            self.nodes[yr].parent = x;
        }
        let xp = self.nodes[x].parent;
        self.nodes[y].parent = xp;
        self.replace_child(xp, x, y);
        self.nodes[y].right = x;
        self.nodes[x].parent = y;
    }

    pub fn insert(&mut self, key: &str) -> bool {
        let mut parent = NIL;
        let mut x = self.root;
        while x != NIL {
            parent = x;
            match Self::compare(key, &self.nodes[x].key) {
                Ordering::Equal => return false,
                Ordering::Less => x = self.nodes[x].left,
                Ordering::Greater => x = self.nodes[x].right,
            }
        }
        let z = self.alloc(key, true, parent);
        if parent == NIL {
            self.root = z;
        } else if Self::less(key, &self.nodes[parent].key) {
            self.nodes[parent].left = z;
        } else {
            self.nodes[parent].right = z;
        }
        self.insert_fix(z);
        true
    }

    fn insert_fix(&mut self, mut z: usize) {
        while self.nodes[self.nodes[z].parent].red {
            let p = self.nodes[z].parent;
            let g = self.nodes[p].parent;
            if p == self.nodes[g].left {
                let uncle = self.nodes[g].right;
                if self.nodes[uncle].red {
                    self.nodes[p].red = false;
                    self.nodes[uncle].red = false;
                    self.nodes[g].red = true;
                    z = g;
                } else {
                    if z == self.nodes[p].right {
                        z = p;
                        self.left_rotate(z);
                    }
                    let p = self.nodes[z].parent;
                    let g = self.nodes[p].parent;
                    self.nodes[p].red = false;
                    self.nodes[g].red = true;
                    self.right_rotate(g);
                }
            } else {
                let uncle = self.nodes[g].left;
                if self.nodes[uncle].red {
                    self.nodes[p].red = false;
                    self.nodes[uncle].red = false;
                    self.nodes[g].red = true;
                    z = g;
                } else {
                    if z == self.nodes[p].left {
                        z = p;
                        self.right_rotate(z);
                    }
                    let p = self.nodes[z].parent;
                    let g = self.nodes[p].parent;
                    self.nodes[p].red = false;
                    self.nodes[g].red = true;
                    self.left_rotate(g);
                }
            }
        }
        let r = self.root;
        self.nodes[r].red = false;
    }

    fn transplant(&mut self, u: usize, v: usize) {
        let up = self.nodes[u].parent;
        self.replace_child(up, u, v);
        self.nodes[v].parent = up;
    }

    pub fn erase(&mut self, key: &str) -> bool {
        let z = self.search(key);
        if z == NIL {
            return false;
        }
        let mut y = z;
        let mut y_was_red = self.nodes[y].red;
        let x;
        if self.nodes[z].left == NIL {
            x = self.nodes[z].right;
            self.transplant(z, x);
        } else if self.nodes[z].right == NIL {
            x = self.nodes[z].left;
            self.transplant(z, x);
        } else {
            y = self.minimum(self.nodes[z].right);
            y_was_red = self.nodes[y].red;
            x = self.nodes[y].right;
            if self.nodes[y].parent == z {
                self.nodes[x].parent = y;
            } else {
                self.transplant(y, x);
                let zr = self.nodes[z].right;
                self.nodes[y].right = zr;
                self.nodes[zr].parent = y;
            }
            self.transplant(z, y);
            let zl = self.nodes[z].left;
            self.nodes[y].left = zl;
            self.nodes[zl].parent = y;
            self.nodes[y].red = self.nodes[z].red;
        }
        self.free.push(z);
        self.nodes[z].key.clear();
        self.count -= 1;
        if !y_was_red {
            self.erase_fix(x);
        }
        self.nodes[NIL].parent = NIL;
        true
    }

    fn erase_fix(&mut self, mut x: usize) {
        while x != self.root && !self.nodes[x].red {
            let p = self.nodes[x].parent;
            if x == self.nodes[p].left {
                let mut w = self.nodes[p].right;
                if self.nodes[w].red {
                    self.nodes[w].red = false;
                    self.nodes[p].red = true;
                    self.left_rotate(p);
                    w = self.nodes[self.nodes[x].parent].right;
                }
                if !self.nodes[self.nodes[w].left].red && !self.nodes[self.nodes[w].right].red {
                    self.nodes[w].red = true;
                    x = self.nodes[x].parent;
                } else {
                    if !self.nodes[self.nodes[w].right].red {
                        let wl = self.nodes[w].left;
                        self.nodes[wl].red = false;
                        self.nodes[w].red = true;
                        self.right_rotate(w);
                        w = self.nodes[self.nodes[x].parent].right;
                    }
                    let p = self.nodes[x].parent;
                    self.nodes[w].red = self.nodes[p].red;
                    self.nodes[p].red = false;
                    let wr = self.nodes[w].right;
                    self.nodes[wr].red = false;
                    self.left_rotate(p);
                    x = self.root;
                }
            } else {
                let mut w = self.nodes[p].left;
                if self.nodes[w].red {
                    self.nodes[w].red = false;
                    self.nodes[p].red = true;
                    self.right_rotate(p);
                    w = self.nodes[self.nodes[x].parent].left;
                }
                if !self.nodes[self.nodes[w].right].red && !self.nodes[self.nodes[w].left].red {
                    self.nodes[w].red = true;
                    x = self.nodes[x].parent;
                } else {
                    if !self.nodes[self.nodes[w].left].red {
                        let wr = self.nodes[w].right;
                        self.nodes[wr].red = false;
                        self.nodes[w].red = true;
                        self.left_rotate(w);
                        w = self.nodes[self.nodes[x].parent].left;
                    }
                    let p = self.nodes[x].parent;
                    self.nodes[w].red = self.nodes[p].red;
                    self.nodes[p].red = false;
                    let wl = self.nodes[w].left;
                    self.nodes[wl].red = false;
                    self.right_rotate(p);
                    x = self.root;
                }
            }
        }
        self.nodes[x].red = false;
    }

    pub fn contains(&self, key: &str) -> bool {
        self.search(key) != NIL
    }

    pub fn min_key(&self) -> Option<&str> {
        (self.root != NIL).then(|| self.nodes[self.minimum(self.root)].key.as_str())
    }

    pub fn max_key(&self) -> Option<&str> {
        (self.root != NIL).then(|| self.nodes[self.maximum(self.root)].key.as_str())
    }

    pub fn next(&self, key: &str) -> Option<&str> {
        let mut x = self.search(key);
        if x == NIL {
            return None;
        }
        if self.nodes[x].right != NIL {
            return Some(&self.nodes[self.minimum(self.nodes[x].right)].key);
        }
        let mut p = self.nodes[x].parent;
        while p != NIL && x == self.nodes[p].right {
            x = p;
            p = self.nodes[p].parent;
        }
        (p != NIL).then(|| self.nodes[p].key.as_str())
    }

    pub fn prev(&self, key: &str) -> Option<&str> {
        let mut x = self.search(key);
        if x == NIL {
            return None;
        }
        if self.nodes[x].left != NIL {
            return Some(&self.nodes[self.maximum(self.nodes[x].left)].key);
        }
        let mut p = self.nodes[x].parent;
        while p != NIL && x == self.nodes[p].left {
            x = p;
            p = self.nodes[p].parent;
        }
        (p != NIL).then(|| self.nodes[p].key.as_str())
    }

    fn label(&self, x: usize) -> String {
        format!(
            "{}({})",
            self.nodes[x].key,
            if self.nodes[x].red { 'R' } else { 'B' }
        )
    }

    fn walk(&self, x: usize, order: u8, out: &mut Vec<String>) {
        if x == NIL {
            return;
        }
        if order == 0 {
            out.push(self.label(x));
        }
        self.walk(self.nodes[x].left, order, out);
        if order == 1 {
            out.push(self.label(x));
        }
        self.walk(self.nodes[x].right, order, out);
        if order == 2 {
            out.push(self.label(x));
        }
    }

    pub fn preorder(&self) -> Vec<String> {
        let mut out = Vec::new();
        self.walk(self.root, 0, &mut out);
        out
    }

    pub fn inorder(&self) -> Vec<String> {
        let mut out = Vec::new();
        self.walk(self.root, 1, &mut out);
        out
    }

    pub fn postorder(&self) -> Vec<String> {
        let mut out = Vec::new();
        self.walk(self.root, 2, &mut out);
        out
    }

    pub fn bfs(&self) -> Vec<String> {
        let mut out = Vec::new();
        let mut queue = VecDeque::new();
        if self.root != NIL {
            queue.push_back(self.root);
        }
        while let Some(x) = queue.pop_front() {
            out.push(self.label(x));
            for c in [self.nodes[x].left, self.nodes[x].right] {
                if c != NIL {
                    queue.push_back(c);
                }
            }
        }
        out
    }

    fn draw(&self, x: usize, prefix: &str, edge: &str, last: bool, out: &mut String) {
        let _ = writeln!(
            out,
            "{prefix}{}{edge}{}",
            if last { "`-- " } else { "|-- " },
            self.label(x)
        );
        let child = format!("{prefix}{}", if last { "    " } else { "|   " });
        let (l, r) = (self.nodes[x].left, self.nodes[x].right);
        if l != NIL {
            self.draw(l, &child, "L: ", r == NIL, out);
        }
        if r != NIL {
            self.draw(r, &child, "R: ", true, out);
        }
    }

    pub fn diagram(&self) -> String {
        if self.root == NIL {
            return "(empty tree)\n".to_string();
        }
        let mut out = format!("{}\n", self.label(self.root));
        let (l, r) = (self.nodes[self.root].left, self.nodes[self.root].right);
        if l != NIL {
            self.draw(l, "", "L: ", r == NIL, &mut out);
        }
        if r != NIL {
            self.draw(r, "", "R: ", true, &mut out);
        }
        out
    }

    fn check(
        &self,
        x: usize,
        low: Option<&str>,
        high: Option<&str>,
        seen: &mut usize,
    ) -> Option<usize> {
        if x == NIL {
            return Some(1);
        }
        *seen += 1;
        let n = &self.nodes[x];
        if low.is_some_and(|l| !Self::less(l, &n.key))
            || high.is_some_and(|h| !Self::less(&n.key, h))
        {
            return None;
        }
        if (n.left != NIL && self.nodes[n.left].parent != x)
            || (n.right != NIL && self.nodes[n.right].parent != x)
        {
            return None;
        }
        if n.red && (self.nodes[n.left].red || self.nodes[n.right].red) {
            return None;
        }
        let lh = self.check(n.left, low, Some(&n.key), seen)?;
        let rh = self.check(n.right, Some(&n.key), high, seen)?;
        (lh == rh).then_some(lh + usize::from(!n.red))
    }

    pub fn is_valid(&self) -> bool {
        if self.nodes[NIL].red || self.nodes[self.root].red {
            return false;
        }
        let mut seen = 0;
        self.check(self.root, None, None, &mut seen).is_some() && seen == self.count
    }

    pub fn height(&self) -> usize {
        fn h(t: &RBTree, x: usize) -> usize {
            if x == NIL {
                0
            } else {
                1 + h(t, t.nodes[x].left).max(h(t, t.nodes[x].right))
            }
        }
        h(self, self.root)
    }

    fn attach(&mut self, key: &str, red: bool) -> Result<(), LoadError> {
        let mut parent = NIL;
        let mut x = self.root;
        while x != NIL {
            parent = x;
            match Self::compare(key, &self.nodes[x].key) {
                Ordering::Equal => {
                    return Err(LoadError::Format("duplicate key in tree data".into()))
                }
                Ordering::Less => x = self.nodes[x].left,
                Ordering::Greater => x = self.nodes[x].right,
            }
        }
        let z = self.alloc(key, red, parent);
        if parent == NIL {
            self.root = z;
        } else if Self::less(key, &self.nodes[parent].key) {
            self.nodes[parent].left = z;
        } else {
            self.nodes[parent].right = z;
        }
        Ok(())
    }

    // Ключи по возрастанию (как keys() в ЛР №1)
    pub fn keys(&self) -> Vec<String> {
        let mut out = Vec::new();
        self.collect_keys(self.root, &mut out);
        out
    }

    fn collect_keys(&self, x: usize, out: &mut Vec<String>) {
        if x == NIL {
            return;
        }
        self.collect_keys(self.nodes[x].left, out);
        out.push(self.nodes[x].key.clone());
        self.collect_keys(self.nodes[x].right, out);
    }

    // Восстановить дерево по меткам прямого обхода "20(B)"; при ошибке дерево не меняется
    fn load_labels(&mut self, labels: &[String]) -> Result<(), LoadError> {
        let mut tree = RBTree::new();
        for label in labels {
            let (key, red) = if let Some(k) = label.strip_suffix("(R)") {
                (k, true)
            } else if let Some(k) = label.strip_suffix("(B)") {
                (k, false)
            } else {
                return Err(LoadError::Format("expected key(R) or key(B)".into()));
            };
            if key.is_empty() {
                return Err(LoadError::Format("expected key(R) or key(B)".into()));
            }
            tree.attach(key, red)?;
        }
        if !tree.is_valid() {
            return Err(LoadError::Format(
                "loaded data is not a valid red-black tree".into(),
            ));
        }
        *self = tree;
        Ok(())
    }
}

impl Container for RBTree {
    fn length(&self) -> usize {
        self.count
    }
    fn clear(&mut self) {
        *self = Self::new();
    }
    fn type_name(&self) -> &'static str {
        "RBTree"
    }
}

impl Sequence for RBTree {
    fn values(&self) -> Vec<String> {
        self.keys()
    }
    fn restore(&mut self, value: &str) {
        self.insert(value);
    }
}

// Дерево сохраняется прямым обходом с цветами (метки 20(B)), чтобы восстановить точную форму
impl Serializable for RBTree {
    fn save_binary(&self, out: &mut dyn Write) -> io::Result<()> {
        let labels = self.preorder();
        codec::write_binary_header(out, self.type_name(), labels.len())?;
        for label in &labels {
            codec::write_string(out, label)?;
        }
        Ok(())
    }

    fn load_binary(&mut self, input: &mut dyn Read) -> Result<(), LoadError> {
        let labels = read_binary_values(input, self.type_name())?;
        self.load_labels(&labels)
    }

    fn save_text(&self, out: &mut dyn Write) -> io::Result<()> {
        codec::write_json(out, self.type_name(), &self.preorder())
    }

    fn load_text(&mut self, input: &mut dyn BufRead) -> Result<(), LoadError> {
        let labels = codec::read_json(input, self.type_name())?;
        self.load_labels(&labels)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn lab1_tree() -> RBTree {
        let mut t = RBTree::new();
        for v in ["10", "20", "30", "15", "5", "25"] {
            assert!(t.insert(v));
        }
        t
    }

    #[test]
    fn insert_balances_like_lab1() {
        let mut t = lab1_tree();
        assert!(!t.insert("15"));
        assert_eq!(t.preorder().join(" "), "20(B) 10(B) 5(R) 15(R) 30(B) 25(R)");
        assert_eq!(t.inorder().join(" "), "5(R) 10(B) 15(R) 20(B) 25(R) 30(B)");
        assert_eq!(
            t.postorder().join(" "),
            "5(R) 15(R) 10(B) 25(R) 30(B) 20(B)"
        );
        assert_eq!(t.bfs().join(" "), "20(B) 10(B) 30(B) 5(R) 15(R) 25(R)");
        assert_eq!(t.values().join(" "), "5 10 15 20 25 30");
        assert!(t.is_valid());
        assert_eq!((t.length(), t.height(), t.type_name()), (6, 3, "RBTree"));
    }

    #[test]
    fn search_min_max_next_prev() {
        let empty = RBTree::default();
        assert_eq!((empty.min_key(), empty.max_key()), (None, None));
        assert!(empty.bfs().is_empty());
        let t = lab1_tree();
        assert!(t.contains("25") && !t.contains("99"));
        assert_eq!((t.min_key(), t.max_key()), (Some("5"), Some("30")));
        assert_eq!((t.next("15"), t.next("20")), (Some("20"), Some("25")));
        assert_eq!(
            (t.prev("15"), t.prev("20"), t.prev("25")),
            (Some("10"), Some("15"), Some("20"))
        );
        assert_eq!((t.next("30"), t.prev("5")), (None, None));
        assert_eq!((t.next("99"), t.prev("99")), (None, None));
    }

    #[test]
    fn erase_like_lab1() {
        let mut t = lab1_tree();
        assert!(!RBTree::new().erase("1"));
        assert!(t.erase("20"));
        assert_eq!(t.preorder().join(" "), "25(B) 10(B) 5(R) 15(R) 30(B)");
        assert!(t.erase("10"));
        assert_eq!(t.preorder().join(" "), "25(B) 15(B) 5(R) 30(B)");
        assert!(!t.erase("99"));
        for k in ["5", "30", "25", "15"] {
            assert!(t.erase(k));
        }
        assert!(t.is_empty() && t.is_valid());
    }

    #[test]
    fn random_operations_keep_invariants() {
        let mut t = RBTree::new();
        let mut x: u32 = 12345;
        for _ in 0..3000 {
            x = x.wrapping_mul(1_103_515_245).wrapping_add(12345);
            let key = ((x >> 8) % 500).to_string();
            if (x >> 4).is_multiple_of(3) {
                t.erase(&key);
            } else {
                t.insert(&key);
            }
        }
        assert!(t.is_valid());
        assert!(t.height() <= 19);
    }

    #[test]
    fn ordering_and_diagram() {
        assert!(RBTree::less("2", "10") && !RBTree::less("10", "2"));
        assert!(RBTree::less("-5", "3") && RBTree::less("99", "abc") && !RBTree::less("abc", "99"));
        assert!(RBTree::less("01", "1") && RBTree::less("apple", "pear"));
        assert!(RBTree::less("1234567890123456789", "a"));
        assert_eq!(RBTree::new().diagram(), "(empty tree)\n");
        let mut t = RBTree::new();
        for v in ["2", "1", "3", "4"] {
            t.insert(v);
        }
        assert_eq!(
            t.diagram(),
            "2(B)\n|-- L: 1(B)\n`-- R: 3(B)\n    `-- R: 4(R)\n"
        );
        let mut l = RBTree::new();
        for v in ["3", "2", "1", "0"] {
            l.insert(v);
        }
        assert_eq!(
            l.diagram(),
            "2(B)\n|-- L: 1(B)\n|   `-- L: 0(R)\n`-- R: 3(B)\n"
        );
    }

    #[test]
    fn clone_clear_restore() {
        let t = lab1_tree();
        let mut c = t.clone();
        c.insert("1");
        assert_eq!(t.length(), 6);
        c.clear();
        assert!(c.is_empty());
        c.restore("z");
        assert!(c.contains("z"));
    }
}
