const { exec } = require('child_process')
const fs = require('fs')
const process = require('process');


function parseResults(output) {
  const jsonString = output.match(/\[.*\]/)
  return jsonString ? JSON.parse(jsonString[0]) : []
}

function createMarkdownTable(results, commandNames) {
  const benchmarks = new Set()

  commandNames.forEach((name) => {
    results[name].forEach((entry) => {
      benchmarks.add(entry.name)
    })
  })

  const benchmarkArray = Array.from(benchmarks)
  const columnWidths = [Math.max(...benchmarkArray.map((b) => b.length), 37)]

  commandNames.forEach((name) => {
    const maxLength = Math.max(
      ...results[name].map((entry) => entry.result.toString().length),
      name.length,
      17
    )
    columnWidths.push(maxLength)
  })

  let table = '| Benchmark (Higher scores are better)  '
  commandNames.forEach((name, index) => {
    table += `| ${name.padEnd(columnWidths[index + 1])} `
  })
  table += '|\n'

  table += '|-' + '-'.repeat(columnWidths[0]) + '-'
  commandNames.forEach((_, index) => {
    table += '|-' + '-'.repeat(columnWidths[index + 1]) + '-'
  })
  table += '|\n'

  benchmarkArray.forEach((benchmark) => {
    const row = [`| ${benchmark.padEnd(columnWidths[0])}`]
    commandNames.forEach((name, index) => {
      const result = results[name].find((entry) => entry.name === benchmark)
      row.push(
        `| ${
          result
            ? result.result.toString().padEnd(columnWidths[index + 1])
            : 'N/A'.padEnd(columnWidths[index + 1])
        }`
      )
    })
    table += `${row.join(' ')} |\n`
  })

  return table
}

function createCSV(results, commandNames) {
  let csv = 'Benchmark,'
  csv += commandNames.join(',') + '\n'

  const benchmarks = new Set()

  commandNames.forEach((name) => {
    results[name].forEach((entry) => {
      benchmarks.add(entry.name)
    })
  })

  benchmarks.forEach((benchmark) => {
    const row = [benchmark]
    commandNames.forEach((name) => {
      const result = results[name].find((entry) => entry.name === benchmark)
      row.push(result ? result.result.toString() : 'N/A')
    })
    csv += `${row.join(',')}\n`
  })

  return csv
}

const commands = [
  //{ name: 'V8 --jitless', command: 'v8 --jitless },
  //{ name: 'V8', command: 'v8' },
  //{ name: 'JSC', command: 'jsc' },
  // Add more engines here
]

const results = {}

function main() {
    commands.forEach(({ name, command }, index) => {
        console.log(`Running ${name}: ${command}`)
        exec(command, (error, stdout, stderr) => {
          if (error) {
            console.error(`Error executing ${name}: ${error.message}`)
            return
          }
      
          const r = parseResults(stdout)
          console.log(name, r)
          results[name] = r
      
          if (Object.keys(results).length === commands.length) {
            const markdownTable = createMarkdownTable(
              results,
              commands.map((cmd) => cmd.name)
            )
            console.log('Markdown Table:')
            console.log(markdownTable)

            /*
            const csvContent = createCSV(
              results,
              commands.map((cmd) => cmd.name)
            )
      
            fs.writeFileSync('benchmark_results.csv', csvContent)
            console.log('CSV file has been saved as benchmark_results.csv')
            */
          }
        })
      })
}

let args = process.argv.slice(2)

for (;;) {
    const [ name, cmd, ...rest ] = args

    if (!name || !cmd)
        break

    commands.push({
        name,
        command: `${cmd} combined.js`
    })

    if (!rest.length)
        break

    args = rest
}

main()
