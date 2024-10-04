## `quickjs-ng/benchmarks`

Benchmarking https://github.com/quickjs-ng/quickjs

### Setup

```bash
git clone https://github.com/quickjs-ng/benchmarks
cd benchmarks

git clone https://github.com/quickjs-ng/quickjs

cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_QJS_LIBC=ON
cmake --build build
```

### Usage

```bash
# sunspider, kraken
./build/run_sunspider_like <folder>/ <filter>

# octane
./build/run_octane
```
Eg:

```bash
./build/run_sunspider_like kraken-1.0/ ai-astar
```

### SunSpider 1.0

```bash
./build/run_sunspider_like sunspider-1.0/
```

### Kraken

```bash
./build/run_sunspider_like kraken-1.1/
```

### Octane

```bash
./build/run_octane
```

### V8

For the V8 benchmarks Node is used as the runner:

```bash
cd v8
node benchmark.js "QuickJS (master)" ../../quickjs/build/qjs-master "QuickJS (this PR)" ../../quickjs/build/qjs "V8 (jitless)" "v8 --jitless"
```

The script takes pairs of "name" and "command" and generates a Markdown table at the end.
