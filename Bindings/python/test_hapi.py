import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from hapi import HapiClient


def main() -> None:
    client = HapiClient()
    assert client.version == "0.1.0"
    assert client.platform == "linux"

    os_info = client.get_os_info()
    assert os_info.kernel_name == "Linux"
    assert len(os_info.hostname) > 0

    cpu = client.get_cpu_info()
    assert cpu.logical_cores > 0

    mem = client.get_memory_info()
    assert mem.total_kb > 0

    user = client.get_user_info()
    assert len(user.username) > 0

    print(
        f"Python SDK OK: {os_info.name} {os_info.version} | "
        f"{cpu.logical_cores} threads | {mem.total_kb // 1024} MiB | user={user.username}"
    )


if __name__ == "__main__":
    main()
