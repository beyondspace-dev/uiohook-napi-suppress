const fs = require('fs')
const path = require('path')
const { spawnSync } = require('child_process')

const repoRoot = path.resolve(__dirname, '..')
const requiredFile = path.join(repoRoot, 'libuiohook', 'src', 'logger.c')
const gitmodulesPath = path.join(repoRoot, '.gitmodules')

if (fs.existsSync(requiredFile)) {
  process.exit(0)
}

if (!fs.existsSync(gitmodulesPath)) {
  console.error('Missing libuiohook sources and no .gitmodules file is available.')
  process.exit(1)
}

const result = spawnSync('git', ['submodule', 'update', '--init', '--recursive'], {
  cwd: repoRoot,
  stdio: 'inherit'
})

if (result.status !== 0) {
  process.exit(result.status || 1)
}

if (!fs.existsSync(requiredFile)) {
  console.error('libuiohook sources are still missing after submodule update.')
  process.exit(1)
}
