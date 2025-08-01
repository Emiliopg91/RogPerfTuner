from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from genericpath import isdir, isfile
import json
from math import ceil, floor
import os
from queue import Empty, Queue
import shutil
from threading import Lock
import time



from dataclasses_json import dataclass_json
from pkg_resources import SOURCE_DIST

ORIGIN_FOLDER = "/var/mnt/Datos"
BACKUP_FOLDER = "/var/mnt/NAS/Public/Backup/Datos"
CFG_FILE = os.path.expanduser(os.path.join("~", ".config", "pyBackup", "config.cfg"))

EXCLUSIONS = [
    "SteamLibrary",
    ".pnpm-store",
    "Desarrollo/Caches",
    "Desarrollo/IDEs",
    "Desarrollo/Toolchain",
    "node_modules",
    ".venv",
    ".Trash-1000",
    "__pycache__",
]


@dataclass_json
@dataclass
class FileEntry:
    mod_time: float
    is_dir: bool
    size: int


def format_size(num_bytes: int) -> str:
    for unidad in ["B", "KB", "MB", "GB", "TB", "PB"]:
        if num_bytes < 1024:
            return f"{num_bytes:.2f} {unidad}"
        num_bytes /= 1024
    return f"{num_bytes:.2f} PB"


def format_time(segundos: int) -> str:
    segundos = round(segundos)
    horas, resto = divmod(segundos, 3600)
    minutos, segundos = divmod(resto, 60)
    return f"{horas:02}:{minutos:02}:{segundos:02}"


def get_tree(output: dict, current: str, root: str = None):
    if root is None:
        root = current
    if not root.endswith("/"):
        root = root + "/"

    try:
        for child in os.listdir(current):
            excluded = False

            for e in EXCLUSIONS:
                if os.path.join(current, child).endswith(e):
                    excluded = True
                    break

            if not excluded:
                path = os.path.join(current, child)

                if os.path.isdir(path):
                    get_tree(output, path, root)
                    output[path.replace(root, "")] = FileEntry(os.path.getmtime(path), os.path.isdir(path), 0)
                else:
                    output[path.replace(root, "")] = FileEntry(
                        os.path.getmtime(path), os.path.isdir(path), os.path.getsize(path)
                    )
    except Exception as e:
        print(f"Error en path {current}: {e}")


print("Listing tree...")
source = {}
get_tree(source, ORIGIN_FOLDER)
print(f"  Found {len(source)} entries in source")

old = {}
try:
    with open(CFG_FILE, "r", encoding="utf-8") as f:
        data = json.load(f)
        data = data[ORIGIN_FOLDER]
        old = {path: FileEntry.from_dict(entry) for path, entry in data.items()}
except Exception as e:
    print(f"[ERROR] {e}")

print("Checking differences...")

actions: dict[str, tuple[FileEntry, str]] = {}


class Stats:
    idx = 0
    end_idx = 0
    written_bytes = 0
    t0 = time.time()
    added = 0
    modified = 0
    removed = 0
    total_bytes = 0
    lock = Lock()
    total_elements = 0

    def get_idx(self):
        with self.lock:
            self.idx = self.idx + 1
            return self.idx

    def update_stats(self, local_meta: FileEntry):
        with self.lock:
            self.end_idx = self.end_idx + 1
            self.written_bytes = self.written_bytes + local_meta.size

            local_elapsed = ceil(time.time() - self.t0)
            local_remaining = (self.total_bytes / (self.written_bytes / local_elapsed)) - local_elapsed
            local_written_bytes = self.written_bytes
        if self.end_idx < self.total_elements:
            print(
                f"    {round((100*local_written_bytes)/self.total_bytes)}% - {format_size(local_written_bytes)}/{format_size(self.total_bytes)} @ {format_size(round(local_written_bytes/local_elapsed))}/s - {format_time(local_elapsed)} - {format_time(local_remaining)}                                                      ",
                end="\r",
            )


stats = Stats()

total_bytes = 0
for f, src_meta in source.items():
    if f not in old:
        stats.added = stats.added + 1
        actions[f] = (src_meta, "+")
        stats.total_bytes = stats.total_bytes + src_meta.size
    else:
        tgt_meta = old[f]
        if not src_meta.is_dir and src_meta.mod_time > tgt_meta.mod_time:
            stats.modified = stats.modified + 1
            actions[f] = (src_meta, "m")
            stats.total_bytes = stats.total_bytes + src_meta.size

for f, dst_meta in old.items():
    if f not in source:
        stats.removed = stats.removed + 1
        stats.total_bytes = stats.total_bytes + dst_meta.size
        actions[f] = (dst_meta, "-")

print(f"  {stats.added} additions")
print(f"  {stats.modified} modifications")
print(f"  {stats.removed} deletions")
print(f"  {format_size(stats.total_bytes)} to write")

actions = dict(sorted(actions.items()))
stats.total_elements = len(actions)

print("")
print("Erasing non-existent folders/files")
for path, (meta, action) in actions.items():
    if action == "-":
        try:
            idx = stats.get_idx()

            # print(f"  [{idx}/{len(actions)}] ({action}) {path}")
            bk_path = os.path.join(BACKUP_FOLDER, path)
            if os.path.exists(bk_path):
                if os.path.isfile(bk_path):
                    os.unlink(bk_path)
                else:
                    shutil.rmtree(bk_path)
        except Exception as e:
            print(f"  ERROR: {e}")
        idx = idx + 1

print("")
print("Creating folder structure")
for path, (meta, action) in actions.items():
    if meta.is_dir and action in ("+", "m"):
        try:
            idx = stats.get_idx()

            # print(f"  [{idx}/{len(actions)}] ({action}) {path}")
            bk_path = os.path.join(BACKUP_FOLDER, path)
            if not os.path.exists(bk_path):
                os.makedirs(bk_path)

            os.utime(bk_path, (meta.mod_time, meta.mod_time))
        except Exception as e:
            print(f"  ERROR: {e}")
        idx = idx + 1


print("")
print("Copying files")


def worker_copy(queue: Queue):
    while True:
        try:
            worker_path, worker_meta = queue.get_nowait()
        except Empty:
            break

        worker_idx = stats.get_idx()

        worker_sr_path = os.path.join(ORIGIN_FOLDER, worker_path)
        worker_bk_path = os.path.join(BACKUP_FOLDER, worker_path)

        # print(f"  [{worker_idx}/{len(actions)}] ({action}) {worker_path}")

        try:
            if not worker_meta.is_dir:
                shutil.copy2(worker_sr_path, worker_bk_path)

            os.utime(worker_bk_path, (worker_meta.mod_time, worker_meta.mod_time))

            stats.update_stats(worker_meta)
        except Exception as e:
            print(f"  ERROR: {e}")
        finally:
            queue.task_done()


small_copy_queue = Queue()
big_copy_queue = Queue()
for path, (meta, action) in actions.items():
    if not meta.is_dir and action in ("+", "m"):
        if meta.size < 100 * 1024 * 1024:
            small_copy_queue.put((path, meta))
        else:
            big_copy_queue.put((path, meta))

with ThreadPoolExecutor(max_workers=10) as executor:
    for _ in range(9):
        executor.submit(worker_copy, small_copy_queue)

    executor.submit(worker_copy, big_copy_queue)

    small_copy_queue.join()
    big_copy_queue.join()

print("")
print(
    f"Backup finished. {format_size(stats.written_bytes)} written in {format_time(time.time()-stats.t0)} ({format_size(stats.written_bytes/(time.time()-stats.t0))}/s)"
)

if not os.path.exists(os.path.dirname(CFG_FILE)):
    os.makedirs(os.path.dirname(CFG_FILE))
data = {}

try:
    with open(CFG_FILE, "r", encoding="utf-8") as f:
        data = json.load(f)
except Exception as e:
    pass

try:
    with open(CFG_FILE, "w", encoding="utf-8") as f:
        data[ORIGIN_FOLDER] = {path: entry.to_dict() for path, entry in source.items()}
        json.dump(data, f, indent=4)
except Exception as e:
    pass
