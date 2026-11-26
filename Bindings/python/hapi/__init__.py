"""Python SDK for API-on-Hyggshi-OS (HAPI) backed by the public C ABI (libhapi_c)."""

from __future__ import annotations

import ctypes
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


def _find_library(lib_path: Optional[str] = None) -> ctypes.CDLL:
    candidates = []
    if lib_path:
        candidates.append(lib_path)
    if env_path := os.environ.get("HAPI_C_LIB"):
        candidates.append(env_path)

    repo_root = Path(__file__).resolve().parents[3]
    candidates.extend([
        str(repo_root / "build" / "libhapi_c.so"),
        "libhapi_c.so",
        "hapi_c.dll",
    ])

    for candidate in candidates:
        try:
            return ctypes.CDLL(candidate)
        except OSError:
            continue
    raise RuntimeError("Could not locate libhapi_c. Build the project or set HAPI_C_LIB.")


class _COsInfo(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char * 128),
        ("version", ctypes.c_char * 64),
        ("id", ctypes.c_char * 64),
        ("codename", ctypes.c_char * 64),
        ("pretty_name", ctypes.c_char * 256),
        ("kernel_name", ctypes.c_char * 64),
        ("kernel_version", ctypes.c_char * 128),
        ("architecture", ctypes.c_char * 64),
        ("hostname", ctypes.c_char * 256),
    ]


class _CCpuInfo(ctypes.Structure):
    _fields_ = [
        ("model_name", ctypes.c_char * 256),
        ("vendor_id", ctypes.c_char * 64),
        ("physical_cores", ctypes.c_uint32),
        ("logical_cores", ctypes.c_uint32),
        ("frequency_mhz", ctypes.c_double),
        ("cache_size_kb", ctypes.c_uint64),
    ]


class _CMemoryInfo(ctypes.Structure):
    _fields_ = [
        ("total_kb", ctypes.c_uint64),
        ("free_kb", ctypes.c_uint64),
        ("available_kb", ctypes.c_uint64),
        ("used_kb", ctypes.c_uint64),
        ("buffers_kb", ctypes.c_uint64),
        ("cached_kb", ctypes.c_uint64),
        ("swap_total_kb", ctypes.c_uint64),
        ("swap_free_kb", ctypes.c_uint64),
        ("swap_used_kb", ctypes.c_uint64),
        ("usage_percent", ctypes.c_double),
    ]


class _CUptimeInfo(ctypes.Structure):
    _fields_ = [
        ("uptime_seconds", ctypes.c_double),
        ("idle_seconds", ctypes.c_double),
    ]


class _CDesktopInfo(ctypes.Structure):
    _fields_ = [
        ("desktop_environment", ctypes.c_char * 128),
        ("session_type", ctypes.c_char * 64),
        ("display", ctypes.c_char * 128),
        ("locale", ctypes.c_char * 64),
        ("is_graphical", ctypes.c_int),
    ]


class _CHardwareInfo(ctypes.Structure):
    _fields_ = [
        ("vendor", ctypes.c_char * 128),
        ("product_name", ctypes.c_char * 128),
        ("product_version", ctypes.c_char * 64),
        ("board_vendor", ctypes.c_char * 128),
        ("board_name", ctypes.c_char * 128),
        ("bios_vendor", ctypes.c_char * 128),
        ("bios_version", ctypes.c_char * 64),
        ("chassis_type", ctypes.c_char * 64),
    ]


class _CUserInfo(ctypes.Structure):
    _fields_ = [
        ("uid", ctypes.c_uint32),
        ("euid", ctypes.c_uint32),
        ("gid", ctypes.c_uint32),
        ("username", ctypes.c_char * 128),
        ("home_dir", ctypes.c_char * 512),
        ("shell", ctypes.c_char * 256),
        ("is_root", ctypes.c_int),
    ]


@dataclass
class OsInfo:
    name: str
    version: str
    id: str
    codename: str
    pretty_name: str
    kernel_name: str
    kernel_version: str
    architecture: str
    hostname: str


@dataclass
class CpuInfo:
    model_name: str
    vendor_id: str
    physical_cores: int
    logical_cores: int
    frequency_mhz: float
    cache_size_kb: int


@dataclass
class MemoryInfo:
    total_kb: int
    free_kb: int
    available_kb: int
    used_kb: int
    buffers_kb: int
    cached_kb: int
    swap_total_kb: int
    swap_free_kb: int
    swap_used_kb: int
    usage_percent: float


@dataclass
class UptimeInfo:
    uptime_seconds: float
    idle_seconds: float


@dataclass
class DesktopInfo:
    desktop_environment: str
    session_type: str
    display: str
    locale: str
    is_graphical: bool


@dataclass
class HardwareInfo:
    vendor: str
    product_name: str
    product_version: str
    board_vendor: str
    board_name: str
    bios_vendor: str
    bios_version: str
    chassis_type: str


@dataclass
class UserInfo:
    uid: int
    euid: int
    gid: int
    username: str
    home_dir: str
    shell: str
    is_root: bool


class HapiClient:
    """High-level Python client wrapping the HAPI C ABI."""

    def __init__(self, lib_path: Optional[str] = None) -> None:
        self._lib = _find_library(lib_path)
        self._lib.hapi_version.restype = ctypes.c_char_p
        self._lib.hapi_platform_name.restype = ctypes.c_char_p
        self._lib.hapi_get_os_info.argtypes = [ctypes.POINTER(_COsInfo)]
        self._lib.hapi_get_cpu_info.argtypes = [ctypes.POINTER(_CCpuInfo)]
        self._lib.hapi_get_memory_info.argtypes = [ctypes.POINTER(_CMemoryInfo)]
        self._lib.hapi_get_uptime_info.argtypes = [ctypes.POINTER(_CUptimeInfo)]
        self._lib.hapi_get_desktop_info.argtypes = [ctypes.POINTER(_CDesktopInfo)]
        self._lib.hapi_get_hardware_info.argtypes = [ctypes.POINTER(_CHardwareInfo)]
        self._lib.hapi_get_user_info.argtypes = [ctypes.POINTER(_CUserInfo)]
        self._lib.hapi_file_exists.argtypes = [ctypes.c_char_p]
        self._lib.hapi_file_read_text.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_size_t),
        ]
        self._lib.hapi_file_write_text.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self._lib.hapi_send_notification.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
        ]

    @property
    def version(self) -> str:
        return self._lib.hapi_version().decode("utf-8")

    @property
    def platform(self) -> str:
        return self._lib.hapi_platform_name().decode("utf-8")

    def get_os_info(self) -> OsInfo:
        raw = _COsInfo()
        rc = self._lib.hapi_get_os_info(ctypes.byref(raw))
        if rc != 0:
            raise RuntimeError(f"hapi_get_os_info failed with code {rc}")
        return OsInfo(
            name=raw.name.decode("utf-8"),
            version=raw.version.decode("utf-8"),
            id=raw.id.decode("utf-8"),
            codename=raw.codename.decode("utf-8"),
            pretty_name=raw.pretty_name.decode("utf-8"),
            kernel_name=raw.kernel_name.decode("utf-8"),
            kernel_version=raw.kernel_version.decode("utf-8"),
            architecture=raw.architecture.decode("utf-8"),
            hostname=raw.hostname.decode("utf-8"),
        )

    def get_cpu_info(self) -> CpuInfo:
        raw = _CCpuInfo()
        rc = self._lib.hapi_get_cpu_info(ctypes.byref(raw))
        if rc != 0:
            raise RuntimeError(f"hapi_get_cpu_info failed with code {rc}")
        return CpuInfo(
            model_name=raw.model_name.decode("utf-8"),
            vendor_id=raw.vendor_id.decode("utf-8"),
            physical_cores=raw.physical_cores,
            logical_cores=raw.logical_cores,
            frequency_mhz=raw.frequency_mhz,
            cache_size_kb=raw.cache_size_kb,
        )

    def get_memory_info(self) -> MemoryInfo:
        raw = _CMemoryInfo()
        rc = self._lib.hapi_get_memory_info(ctypes.byref(raw))
        if rc != 0:
            raise RuntimeError(f"hapi_get_memory_info failed with code {rc}")
        return MemoryInfo(
            total_kb=raw.total_kb,
            free_kb=raw.free_kb,
            available_kb=raw.available_kb,
            used_kb=raw.used_kb,
            buffers_kb=raw.buffers_kb,
            cached_kb=raw.cached_kb,
            swap_total_kb=raw.swap_total_kb,
            swap_free_kb=raw.swap_free_kb,
            swap_used_kb=raw.swap_used_kb,
            usage_percent=raw.usage_percent,
        )

    def get_uptime_info(self) -> UptimeInfo:
        raw = _CUptimeInfo()
        rc = self._lib.hapi_get_uptime_info(ctypes.byref(raw))
        if rc != 0:
            raise RuntimeError(f"hapi_get_uptime_info failed with code {rc}")
        return UptimeInfo(uptime_seconds=raw.uptime_seconds, idle_seconds=raw.idle_seconds)

    def get_desktop_info(self) -> DesktopInfo:
        raw = _CDesktopInfo()
        rc = self._lib.hapi_get_desktop_info(ctypes.byref(raw))
        if rc != 0:
            raise RuntimeError(f"hapi_get_desktop_info failed with code {rc}")
        return DesktopInfo(
            desktop_environment=raw.desktop_environment.decode("utf-8"),
            session_type=raw.session_type.decode("utf-8"),
            display=raw.display.decode("utf-8"),
            locale=raw.locale.decode("utf-8"),
            is_graphical=bool(raw.is_graphical),
        )

    def get_hardware_info(self) -> HardwareInfo:
        raw = _CHardwareInfo()
        rc = self._lib.hapi_get_hardware_info(ctypes.byref(raw))
        if rc != 0:
            raise RuntimeError(f"hapi_get_hardware_info failed with code {rc}")
        return HardwareInfo(
            vendor=raw.vendor.decode("utf-8"),
            product_name=raw.product_name.decode("utf-8"),
            product_version=raw.product_version.decode("utf-8"),
            board_vendor=raw.board_vendor.decode("utf-8"),
            board_name=raw.board_name.decode("utf-8"),
            bios_vendor=raw.bios_vendor.decode("utf-8"),
            bios_version=raw.bios_version.decode("utf-8"),
            chassis_type=raw.chassis_type.decode("utf-8"),
        )

    def get_user_info(self) -> UserInfo:
        raw = _CUserInfo()
        rc = self._lib.hapi_get_user_info(ctypes.byref(raw))
        if rc != 0:
            raise RuntimeError(f"hapi_get_user_info failed with code {rc}")
        return UserInfo(
            uid=raw.uid,
            euid=raw.euid,
            gid=raw.gid,
            username=raw.username.decode("utf-8"),
            home_dir=raw.home_dir.decode("utf-8"),
            shell=raw.shell.decode("utf-8"),
            is_root=bool(raw.is_root),
        )

    def file_exists(self, path: str) -> bool:
        return bool(self._lib.hapi_file_exists(path.encode("utf-8")))

    def read_text(self, path: str, max_bytes: int = 65536) -> str:
        buf = ctypes.create_string_buffer(max_bytes)
        out_len = ctypes.c_size_t(0)
        rc = self._lib.hapi_file_read_text(
            path.encode("utf-8"), buf, max_bytes, ctypes.byref(out_len)
        )
        if rc != 0:
            raise RuntimeError(f"hapi_file_read_text failed with code {rc}")
        return buf.value.decode("utf-8")

    def write_text(self, path: str, content: str) -> None:
        rc = self._lib.hapi_file_write_text(path.encode("utf-8"), content.encode("utf-8"))
        if rc != 0:
            raise RuntimeError(f"hapi_file_write_text failed with code {rc}")
