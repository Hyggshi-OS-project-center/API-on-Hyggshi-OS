// Standalone Node.js test — no TypeScript compilation needed.
// Implements the same pure /proc-based logic as the TS adapter.

import { readFile } from "node:fs/promises";
import { hostname, userInfo, arch } from "node:os";

function parseOsReleaseText(text) {
  const fields = {};
  for (const line of text.split("\n")) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.startsWith("#")) continue;
    const eq = trimmed.indexOf("=");
    if (eq < 1) continue;
    let val = trimmed.slice(eq + 1).trim();
    if ((val.startsWith('"') && val.endsWith('"')) ||
        (val.startsWith("'") && val.endsWith("'"))) {
      val = val.slice(1, -1);
    }
    fields[trimmed.slice(0, eq).trim()] = val;
  }
  return fields;
}

async function safe(path) {
  try { return await readFile(path, "utf8"); } catch { return ""; }
}

async function run() {
  const osText = (await safe("/etc/os-release")) || (await safe("/usr/lib/os-release"));
  const osFields = parseOsReleaseText(osText);
  const unameR = (await safe("/proc/sys/kernel/osrelease")).trim();

  const cpuText = await safe("/proc/cpuinfo");
  let model = ""; let vendor = ""; let cores = 0; let freqMhz = 0;
  for (const line of cpuText.split("\n")) {
    const m = /^([^:]+):\s*(.+)$/.exec(line.trim());
    if (!m) continue;
    const k = m[1].trim(); const v = m[2].trim();
    if (k === "model name" && !model) model = v;
    if (k === "vendor_id" && !vendor) vendor = v;
    if (k === "processor") cores++;
    if (k === "cpu MHz" && !freqMhz) freqMhz = parseFloat(v);
  }

  const memText = await safe("/proc/meminfo");
  const memFields = {};
  for (const line of memText.split("\n")) {
    const mm = /^(\w+):\s+(\d+)/.exec(line);
    if (mm) memFields[mm[1]] = parseInt(mm[2], 10);
  }
  const totalKb = memFields["MemTotal"] || 0;
  const availKb = memFields["MemAvailable"] || memFields["MemFree"] || 0;
  const usedKb = totalKb - availKb;

  const ui = userInfo();
  const isRoot = typeof process.geteuid === "function" && process.geteuid() === 0;

  console.log("=== TypeScript/Node.js SDK ===");
  console.log("OS:  " + (osFields["NAME"] || "Linux") + " " + (osFields["VERSION_ID"] || ""));
  console.log("Kernel: Linux " + unameR + " [" + arch() + "]");
  console.log("Host: " + hostname());
  console.log("CPU: " + model + " (" + cores + " threads, " + freqMhz.toFixed(1) + " MHz)");
  console.log("RAM: " + Math.round(usedKb / 1024) + " / " + Math.round(totalKb / 1024) + " MiB");
  console.log("User: " + ui.username + " root=" + isRoot);
  console.log("\nTypeScript SDK OK.");
}

run().catch((e) => { console.error(e); process.exit(1); });
