/**
 * TypeScript Node.js SDK for API-on-Hyggshi-OS (HAPI)
 * Implements the HapiAdapter interface via koffi (N-API FFI) or native child_process fallback.
 *
 * Production usage: `npm install koffi` then import this module.
 * Fallback (no koffi): reads /proc and /etc directly in pure Node.js for read-only OS info.
 */

export interface OsInfo {
  name: string;
  version: string;
  id: string;
  codename: string;
  prettyName: string;
  kernelName: string;
  kernelVersion: string;
  architecture: string;
  hostname: string;
}

export interface CpuInfo {
  modelName: string;
  vendorId: string;
  physicalCores: number;
  logicalCores: number;
  frequencyMhz: number;
  cacheSizeKb: number;
}

export interface MemoryInfo {
  totalKb: number;
  freeKb: number;
  availableKb: number;
  usedKb: number;
  buffersKb: number;
  cachedKb: number;
  swapTotalKb: number;
  swapFreeKb: number;
  swapUsedKb: number;
  usagePercent: number;
}

export interface DesktopInfo {
  desktopEnvironment: string;
  sessionType: string;
  display: string;
  locale: string;
  isGraphical: boolean;
}

export interface UserInfo {
  uid: number;
  euid: number;
  gid: number;
  username: string;
  homeDir: string;
  shell: string;
  isRoot: boolean;
}

export interface HapiAdapter {
  getOsInfo(): Promise<OsInfo>;
  getCpuInfo(): Promise<CpuInfo>;
  getMemoryInfo(): Promise<MemoryInfo>;
  getDesktopInfo(): Promise<DesktopInfo>;
  getUserInfo(): Promise<UserInfo>;
}

// ---------------------------------------------------------------------------
// Pure Node.js fallback adapter (Linux only, reads /proc directly)
// ---------------------------------------------------------------------------

import { readFile, access } from "node:fs/promises";
import { constants } from "node:fs";
import { hostname, userInfo, platform, arch } from "node:os";

function parseOsReleaseText(text: string): Record<string, string> {
  const fields: Record<string, string> = {};
  for (const line of text.split("\n")) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.startsWith("#")) continue;
    const eq = trimmed.indexOf("=");
    if (eq < 1) continue;
    const key = trimmed.slice(0, eq).trim();
    let val = trimmed.slice(eq + 1).trim();
    if ((val.startsWith('"') && val.endsWith('"')) ||
        (val.startsWith("'") && val.endsWith("'"))) {
      val = val.slice(1, -1);
    }
    fields[key] = val;
  }
  return fields;
}

function parseMeminfo(text: string): Record<string, number> {
  const fields: Record<string, number> = {};
  for (const line of text.split("\n")) {
    const m = /^(\w+):\s+(\d+)/.exec(line);
    if (m) fields[m[1]] = parseInt(m[2], 10);
  }
  return fields;
}

function parseCpuinfo(text: string): {
  modelName: string; vendorId: string;
  logicalCores: number; frequencyMhz: number; cacheSizeKb: number;
} {
  let modelName = "";
  let vendorId = "";
  let logicalCores = 0;
  let frequencyMhz = 0;
  let cacheSizeKb = 0;
  for (const line of text.split("\n")) {
    const m = /^([^:]+):\s*(.+)$/.exec(line.trim());
    if (!m) continue;
    const key = m[1].trim();
    const val = m[2].trim();
    if (key === "model name" && !modelName) modelName = val;
    if (key === "vendor_id" && !vendorId) vendorId = val;
    if (key === "processor") logicalCores++;
    if (key === "cpu MHz" && !frequencyMhz) frequencyMhz = parseFloat(val);
    if (key === "cache size" && !cacheSizeKb) {
      cacheSizeKb = parseInt(val, 10);
    }
  }
  return { modelName, vendorId, logicalCores, frequencyMhz, cacheSizeKb };
}

async function readTextSafe(path: string): Promise<string> {
  try {
    return await readFile(path, "utf8");
  } catch {
    return "";
  }
}

export class LinuxProcAdapter implements HapiAdapter {
  async getOsInfo(): Promise<OsInfo> {
    const text = (await readTextSafe("/etc/os-release")) ||
                 (await readTextSafe("/usr/lib/os-release"));
    const fields = parseOsReleaseText(text);

    const os = await import("node:os");
    const unameR = await readTextSafe("/proc/sys/kernel/osrelease");

    return {
      name: fields["NAME"] ?? platform(),
      version: fields["VERSION_ID"] ?? fields["VERSION"] ?? "",
      id: fields["ID"] ?? "",
      codename: fields["VERSION_CODENAME"] ?? fields["UBUNTU_CODENAME"] ?? "",
      prettyName: fields["PRETTY_NAME"] ?? "",
      kernelName: "Linux",
      kernelVersion: unameR.trim() || os.release(),
      architecture: arch(),
      hostname: hostname(),
    };
  }

  async getCpuInfo(): Promise<CpuInfo> {
    const text = await readTextSafe("/proc/cpuinfo");
    const parsed = parseCpuinfo(text);
    return {
      modelName: parsed.modelName,
      vendorId: parsed.vendorId,
      physicalCores: parsed.logicalCores,
      logicalCores: parsed.logicalCores,
      frequencyMhz: parsed.frequencyMhz,
      cacheSizeKb: parsed.cacheSizeKb,
    };
  }

  async getMemoryInfo(): Promise<MemoryInfo> {
    const text = await readTextSafe("/proc/meminfo");
    const f = parseMeminfo(text);
    const total = f["MemTotal"] ?? 0;
    const free = f["MemFree"] ?? 0;
    const available = f["MemAvailable"] ?? free;
    const used = total - available;
    const swapTotal = f["SwapTotal"] ?? 0;
    const swapFree = f["SwapFree"] ?? 0;
    return {
      totalKb: total,
      freeKb: free,
      availableKb: available,
      usedKb: used,
      buffersKb: f["Buffers"] ?? 0,
      cachedKb: f["Cached"] ?? 0,
      swapTotalKb: swapTotal,
      swapFreeKb: swapFree,
      swapUsedKb: swapTotal - swapFree,
      usagePercent: total > 0 ? (used / total) * 100 : 0,
    };
  }

  async getDesktopInfo(): Promise<DesktopInfo> {
    const env = process.env;
    const sessionType =
      env["XDG_SESSION_TYPE"] ??
      (env["WAYLAND_DISPLAY"] ? "wayland" : env["DISPLAY"] ? "x11" : "tty");
    const display = env["WAYLAND_DISPLAY"] ?? env["DISPLAY"] ?? "";
    return {
      desktopEnvironment:
        env["XDG_CURRENT_DESKTOP"] ?? env["XDG_SESSION_DESKTOP"] ?? env["DESKTOP_SESSION"] ?? "",
      sessionType,
      display,
      locale: env["LC_ALL"] ?? env["LANG"] ?? "",
      isGraphical: !!env["WAYLAND_DISPLAY"] || !!env["DISPLAY"],
    };
  }

  async getUserInfo(): Promise<UserInfo> {
    const ui = userInfo();
    return {
      uid: process.getuid?.() ?? 0,
      euid: process.geteuid?.() ?? 0,
      gid: process.getgid?.() ?? 0,
      username: ui.username,
      homeDir: ui.homedir,
      shell: ui.shell ?? "",
      isRoot: (process.geteuid?.() ?? 1) === 0,
    };
  }
}

/** Returns a LinuxProcAdapter (the only adapter available without koffi). */
export function createHapiClient(): HapiAdapter {
  return new LinuxProcAdapter();
}
