import { createHapiClient } from "./src/index.js";

async function main() {
  const client = createHapiClient();

  const os = await client.getOsInfo();
  console.log("=== OS Info ===");
  console.log("Name:    " + os.name + " " + os.version);
  console.log("Kernel:  " + os.kernelName + " " + os.kernelVersion + " [" + os.architecture + "]");
  console.log("Host:    " + os.hostname);

  const cpu = await client.getCpuInfo();
  console.log("\n=== CPU Info ===");
  console.log("Model:   " + cpu.modelName);
  console.log("Cores:   " + cpu.physicalCores + " physical / " + cpu.logicalCores + " logical");
  console.log("Freq:    " + cpu.frequencyMhz.toFixed(1) + " MHz");

  const mem = await client.getMemoryInfo();
  console.log("\n=== Memory Info ===");
  console.log("Total:   " + Math.round(mem.totalKb / 1024) + " MiB");
  console.log("Used:    " + Math.round(mem.usedKb / 1024) + " MiB (" + mem.usagePercent.toFixed(1) + "%)");

  const desk = await client.getDesktopInfo();
  console.log("\n=== Desktop Info ===");
  console.log("DE:      " + (desk.desktopEnvironment || "N/A"));
  console.log("Session: " + desk.sessionType);
  console.log("Graphical: " + desk.isGraphical);

  const user = await client.getUserInfo();
  console.log("\n=== User Info ===");
  console.log("User:    " + user.username + " (uid=" + user.uid + ")");
  console.log("Home:    " + user.homeDir);
  console.log("Root:    " + user.isRoot);
}

main().catch((err) => { console.error(err); process.exit(1); });
