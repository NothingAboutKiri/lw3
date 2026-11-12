use lab3::{
    Array, ChainHashTable, DoublyList, Format, HashTable, OpenHashTable, Probing, Queue, RBTree,
    Serializable, SinglyList, Stack,
};
use std::hint::black_box;
use std::time::{Duration, Instant};

fn keys(n: usize) -> Vec<String> {
    let mut x: u32 = 2_463_534_242;
    (0..n)
        .map(|_| {
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            (x % 1_000_000).to_string()
        })
        .collect()
}

fn measure(name: &str, n: usize, mut run: impl FnMut()) {
    let mut times = Vec::new();
    let budget = Duration::from_millis(300);
    let start = Instant::now();
    while times.len() < 5 || (start.elapsed() < budget && times.len() < 200) {
        let t = Instant::now();
        run();
        times.push(t.elapsed());
    }
    times.sort();
    let median = times[times.len() / 2];
    #[allow(clippy::cast_precision_loss)]
    let per_op = median.as_nanos() as f64 / n as f64;
    println!(
        "{name:<28} {n:>7} {:>12.1} us {per_op:>10.1} ns/op  ({} runs)",
        median.as_secs_f64() * 1e6,
        times.len()
    );
}

type MakeTable = fn() -> Box<dyn HashTable>;

fn bench_linear(n: usize, ks: &[String]) {
    measure("Array/push_back", n, || {
        let mut a = Array::new();
        for k in ks {
            a.push_back(k);
        }
        black_box(a);
    });
    if n <= 10_000 {
        measure("Array/insert_front", n, || {
            let mut a = Array::new();
            for k in ks {
                a.insert(0, k);
            }
            black_box(a);
        });
    }
    measure("SinglyList/push_front", n, || {
        let mut l = SinglyList::new();
        for k in ks {
            l.push_front(k);
        }
        black_box(l);
    });
    let mut filled = SinglyList::new();
    for k in ks {
        filled.push_back(k);
    }
    measure("SinglyList/contains(miss)", n, || {
        black_box(filled.contains("missing"));
    });
    measure("DoublyList/push+pop_back", n, || {
        let mut l = DoublyList::new();
        for k in ks {
            l.push_back(k);
        }
        while l.pop_back().is_some() {}
        black_box(l);
    });
    measure("Stack/push+pop", n, || {
        let mut s = Stack::new();
        for k in ks {
            s.push(k);
        }
        while s.pop().is_some() {}
        black_box(s);
    });
    measure("Queue/push+pop", n, || {
        let mut q = Queue::new();
        for k in ks {
            q.push(k);
        }
        while q.pop().is_some() {}
        black_box(q);
    });
}

fn bench_tables(n: usize, ks: &[String]) {
    let makers: [(&str, MakeTable); 3] = [
        ("Chain", || Box::new(ChainHashTable::new())),
        ("Open/linear", || {
            Box::new(OpenHashTable::new(Probing::Linear, 11))
        }),
        ("Open/double", || {
            Box::new(OpenHashTable::new(Probing::Double, 11))
        }),
    ];
    for (name, make) in makers {
        measure(&format!("{name}/put"), n, || {
            let mut t = make();
            for k in ks {
                t.put(k, k);
            }
            black_box(t);
        });
        let mut t = make();
        for k in ks {
            t.put(k, k);
        }
        measure(&format!("{name}/get"), n, || {
            for k in ks {
                black_box(t.get(k));
            }
        });
    }
}

fn bench_tree_and_serialization(n: usize, ks: &[String]) {
    measure("RBTree/insert", n, || {
        let mut t = RBTree::new();
        for k in ks {
            t.insert(k);
        }
        black_box(t);
    });
    let mut tree = RBTree::new();
    for k in ks {
        tree.insert(k);
    }
    measure("RBTree/contains", n, || {
        for k in ks {
            black_box(tree.contains(k));
        }
    });
    let mut arr = Array::new();
    for k in ks {
        arr.push_back(k);
    }
    for (fname, format) in [("binary", Format::Binary), ("text", Format::Text)] {
        measure(&format!("Serialize/{fname}"), n, || {
            let mut buf = Vec::new();
            arr.save(&mut buf, format).unwrap();
            let mut back = Array::new();
            back.load(&mut buf.as_slice(), format).unwrap();
            black_box(back);
        });
    }
}

fn main() {
    println!(
        "{:<28} {:>7} {:>15} {:>15}",
        "benchmark", "n", "median", "per element"
    );
    for n in [1_000, 10_000, 100_000] {
        let ks = keys(n);
        bench_linear(n, &ks);
        bench_tables(n, &ks);
        bench_tree_and_serialization(n, &ks);
        println!();
    }
}
