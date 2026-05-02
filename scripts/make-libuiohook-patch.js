const fs = require('fs')
const path = require('path')
const { spawnSync } = require('child_process')

const repoRoot = path.resolve(__dirname, '..')
const patchPath = path.join(repoRoot, 'src', 'libuiohook.patch')

const result = spawnSync('git', ['-C', 'libuiohook', 'diff', '--cached'], {
  cwd: repoRoot,
  encoding: 'utf8',
})

if (result.status !== 0) {
  process.stderr.write(result.stderr || '')
  process.exit(result.status || 1)
}

fs.writeFileSync(patchPath, result.stdout, 'utf8')
