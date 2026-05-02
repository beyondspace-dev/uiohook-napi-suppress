const fs = require('fs')
const path = require('path')
const { spawnSync } = require('child_process')

const repoRoot = path.resolve(__dirname, '..')
const submodulePath = path.join(repoRoot, 'libuiohook')
const requiredFile = path.join(submodulePath, 'src', 'logger.c')
const patchPath = path.join(repoRoot, 'src', 'libuiohook.patch')
const gitmodulesPath = path.join(repoRoot, '.gitmodules')

function runGit(args, options = {}) {
  const result = spawnSync('git', args, {
    cwd: repoRoot,
    stdio: 'inherit',
    ...options,
  })

  if (result.status !== 0) {
    process.exit(result.status || 1)
  }
}

function runGitCapture(args, options = {}) {
  return spawnSync('git', args, {
    cwd: repoRoot,
    encoding: 'utf8',
    ...options,
  })
}

if (!fs.existsSync(requiredFile)) {
  if (!fs.existsSync(gitmodulesPath)) {
    console.error('Missing libuiohook sources and no .gitmodules file is available.')
    process.exit(1)
  }

  runGit(['submodule', 'update', '--init', '--recursive'])
}

if (!fs.existsSync(requiredFile)) {
  console.error('libuiohook sources are still missing after submodule update.')
  process.exit(1)
}

if (!fs.existsSync(patchPath)) {
  console.error('Missing src/libuiohook.patch.')
  process.exit(1)
}

const checkResult = runGitCapture(['-C', 'libuiohook', 'apply', '--check', '../src/libuiohook.patch'])
if (checkResult.status === 0) {
  runGit(['-C', 'libuiohook', 'apply', '../src/libuiohook.patch'])
  process.exit(0)
}

const reverseCheckResult = runGitCapture(['-C', 'libuiohook', 'apply', '--reverse', '--check', '../src/libuiohook.patch'])
if (reverseCheckResult.status === 0) {
  process.exit(0)
}

console.error(checkResult.stderr || 'Failed to apply src/libuiohook.patch.')
process.exit(checkResult.status || 1)
