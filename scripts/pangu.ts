#!/usr/bin/env npx tsx
/**
 * 在中英文/数字相邻处插入空格（pangu 风格排版），改善中文排版观感。
 *
 * 用法:
 *   npx tsx scripts/pangu.ts              # 写回 content/
 *   npx tsx scripts/pangu.ts --check      # 仅预览，不写文件
 *   npx tsx scripts/pangu.ts --dir docs --verbose
 */

import { readFile, writeFile } from "node:fs/promises"
import path from "node:path"
import { globby } from "globby"

/** 仅汉字（不含 CJK 标点），避免「16、32」变成「16 、 32」 */
const HAN = "\\u3400-\\u4DBF\\u4E00-\\u9FFF\\uF900-\\uFAFF"
const HAN_CLASS = `[${HAN}]`
const ALNUM = "[A-Za-z0-9]"
const HAN_TO_ALNUM = new RegExp(`(${HAN_CLASS})(${ALNUM})`, "gu")
const ALNUM_TO_HAN = new RegExp(`(${ALNUM})(${HAN_CLASS})`, "gu")

type Zone = "body" | "frontmatter" | "code_fence" | "inline_code" | "link_url" | "raw_url" | "angle_url"
type LineZone = Exclude<Zone, "frontmatter" | "code_fence">

function addCjkSpaces(text: string): string {
  let prev = ""
  let out = text
  while (out !== prev) {
    prev = out
    out = out.replace(HAN_TO_ALNUM, "$1 $2").replace(ALNUM_TO_HAN, "$1 $2")
  }
  return out
}

function isWhitespace(ch: string): boolean {
  return ch === " " || ch === "\t"
}

/** 从 index 起若像 URL，返回 URL 结束位置（不含），否则返回 index */
function scanUrlEnd(line: string, start: number): number {
  let i = start
  if (line.startsWith("https://", i) || line.startsWith("http://", i)) {
    i += line.startsWith("https://", i) ? 8 : 7
    while (i < line.length) {
      const ch = line[i]
      if (isWhitespace(ch) || ch === ")" || ch === "]" || ch === ">") break
      i++
    }
    return i
  }
  return start
}

function processLine(line: string, zone: Zone): { line: string; zone: Zone } {
  if (zone === "frontmatter" || zone === "code_fence") {
    return { line, zone }
  }

  let out = ""
  let i = 0
  let current: LineZone = zone === "inline_code" ? "inline_code" : "body"
  let buffer = ""

  const flushBuffer = (asZone: Zone) => {
    if (buffer === "") return
    out += asZone === "body" ? addCjkSpaces(buffer) : buffer
    buffer = ""
  }

  while (i < line.length) {
    const ch = line[i]

    if (current === "inline_code") {
      buffer += ch
      if (ch === "`") {
        flushBuffer("inline_code")
        current = "body"
      }
      i++
      continue
    }

    if (current === "link_url") {
      buffer += ch
      if (ch === ")") {
        flushBuffer("link_url")
        current = "body"
      }
      i++
      continue
    }

    if (current === "angle_url") {
      buffer += ch
      if (ch === ">") {
        flushBuffer("angle_url")
        current = "body"
      }
      i++
      continue
    }

    // body（裸 URL 在下方一次性 slice，不经 raw_url 状态）
    if (ch === "`") {
      flushBuffer("body")
      buffer = "`"
      current = "inline_code"
      i++
      continue
    }

    if (ch === "[") {
      flushBuffer("body")
      out += ch
      i++
      while (i < line.length) {
        const c = line[i]
        if (c === "]" && i + 1 < line.length && line[i + 1] === "(") {
          out += addCjkSpaces(buffer)
          buffer = ""
          out += "]("
          i += 2
          current = "link_url"
          break
        }
        if (c === "]") {
          out += addCjkSpaces(buffer) + c
          buffer = ""
          i++
          break
        }
        buffer += c
        i++
      }
      if (current !== "link_url" && buffer) {
        out += addCjkSpaces(buffer)
        buffer = ""
      }
      continue
    }

    if (ch === "<" && (line.startsWith("https://", i + 1) || line.startsWith("http://", i + 1))) {
      flushBuffer("body")
      buffer = "<"
      current = "angle_url"
      i++
      continue
    }

    const urlEnd = scanUrlEnd(line, i)
    if (urlEnd > i) {
      flushBuffer("body")
      buffer = line.slice(i, urlEnd)
      flushBuffer("raw_url")
      i = urlEnd
      continue
    }

    buffer += ch
    i++
  }

  if (buffer) {
    flushBuffer(current)
  }

  return { line: out, zone: current === "inline_code" ? "inline_code" : "body" }
}

function processMarkdown(source: string): string {
  const lines = source.split("\n")
  let zone: Zone = "body"
  let fenceMarker = ""
  let inFrontmatter = false
  let sawFrontmatterOpen = false
  const out: string[] = []

  for (const line of lines) {
    if (!sawFrontmatterOpen && line.trim() === "---") {
      sawFrontmatterOpen = true
      inFrontmatter = true
      out.push(line)
      continue
    }
    if (inFrontmatter) {
      out.push(line)
      if (line.trim() === "---") inFrontmatter = false
      continue
    }

    const trimmed = line.trimStart()
    if (zone === "body" && (trimmed.startsWith("```") || trimmed.startsWith("~~~"))) {
      const marker = trimmed.slice(0, 3)
      if (fenceMarker === "") {
        fenceMarker = marker
        zone = "code_fence"
        out.push(line)
        continue
      }
      if (trimmed.startsWith(fenceMarker)) {
        fenceMarker = ""
        zone = "body"
        out.push(line)
        continue
      }
    }
    if (zone === "code_fence") {
      out.push(line)
      continue
    }
    const result = processLine(line, zone)
    zone = result.zone === "inline_code" ? "body" : result.zone
    out.push(result.line)
  }

  return out.join("\n")
}

async function main() {
  const args = process.argv.slice(2)
  const checkOnly = args.includes("--check")
  const verbose = args.includes("--verbose")
  const dirIdx = args.indexOf("--dir")
  const targetDir = dirIdx >= 0 ? args[dirIdx + 1] : "content"
  if (!targetDir) {
    console.error("缺少 --dir 参数值")
    process.exit(2)
  }

  const root = path.resolve(process.cwd(), targetDir)
  const files = await globby("**/*.md", { cwd: root, absolute: true })
  if (files.length === 0) {
    console.log(`未在 ${root} 下找到 .md 文件`)
    process.exit(0)
  }

  let changedFiles = 0
  let changedLines = 0

  for (const file of files.sort()) {
    const original = await readFile(file, "utf8")
    const updated = processMarkdown(original)
    if (original === updated) continue

    const rel = path.relative(process.cwd(), file)
    const origLines = original.split("\n")
    const newLines = updated.split("\n")
    const lineDiffs: number[] = []
    const max = Math.max(origLines.length, newLines.length)
    for (let i = 0; i < max; i++) {
      if (origLines[i] !== newLines[i]) lineDiffs.push(i + 1)
    }

    changedFiles++
    changedLines += lineDiffs.length

    if (verbose) {
      console.log(`\n${rel}（${lineDiffs.length} 行）`)
      for (const ln of lineDiffs.slice(0, 20)) {
        console.log(`  L${ln}:`)
        console.log(`    - ${origLines[ln - 1]}`)
        console.log(`    + ${newLines[ln - 1]}`)
      }
      if (lineDiffs.length > 20) {
        console.log(`  … 另有 ${lineDiffs.length - 20} 行`)
      }
    } else {
      console.log(`${rel}：${lineDiffs.length} 行已调整`)
    }

    if (!checkOnly) {
      await writeFile(file, updated, "utf8")
    }
  }

  if (changedFiles === 0) {
    console.log("无需调整。")
    process.exit(0)
  }

  if (checkOnly) {
    console.log(`\n共 ${changedFiles} 个文件、${changedLines} 行待调整（未写入，使用了 --check）。`)
    process.exit(1)
  }

  console.log(`\n已更新 ${changedFiles} 个文件（${changedLines} 行）。`)
}

main().catch((err) => {
  console.error(err)
  process.exit(2)
})
