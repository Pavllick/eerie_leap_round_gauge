# CTF Tracing on native_sim

Kernel-level tracing (thread switches, mutex/semaphore/work-queue lifecycle,
GPIO calls, sleeps, ISRs, ...) via Zephyr's `TRACING` subsystem, encoded as
CTF (Common Trace Format) and written to a file by the POSIX backend when
running under `native_sim`.

## Enabling it

In [`app/boards/native_sim.conf`](../../boards/native_sim.conf):

```
CONFIG_TRACING=y
CONFIG_TRACING_CTF=y
# POSIX backend requires synchronous tracing (TRACING_BACKEND_RAM is used silently otherwise)
CONFIG_TRACING_SYNC=y
CONFIG_TRACING_BACKEND_POSIX=y
```

## Capturing a trace

Run the executable from the directory you want the trace file written to
(it defaults to `channel0_0` in the current working directory):

```bash
./build/zephyr/zephyr.exe
```

Let it run, then stop it with `Ctrl-C`. Optionally choose a different
output path/name instead of the default:

```bash
./build/zephyr/zephyr.exe --trace-file=my_trace/channel0_0
```

## Decoding

The CTF metadata schema is a static template, not generated per run — copy
it from the Zephyr tree next to your captured stream:

```bash
mkdir -p my_trace
cp channel0_0 my_trace/            # skip if already written there directly
cp $ZEPHYR_BASE/subsys/tracing/ctf/tsdl/metadata my_trace/
```

Then open it with any CTF-aware tool, e.g. [babeltrace2](https://babeltrace.org/):

```bash
babeltrace2 my_trace/
```

Trace Compass (GUI) or `scripts/tracing/ctf_top.py` also accept the same
`my_trace/` directory (metadata + channel file together = one CTF trace).

## Notes

- `CONFIG_TRACING_BACKEND_RAM` (Zephyr's default choice) buffers events in a
  RAM ring instead of a file — useful on real hardware where you'd dump the
  buffer with a debugger, not applicable here since `native_sim` has direct
  filesystem access via the POSIX backend.
- Per-subsystem tracing points (`CONFIG_TRACING_THREAD`, `_WORK`, `_MUTEX`,
  `_GPIO`, etc.) are separate toggles under `subsys/tracing/Kconfig` — only
  enable the ones you need to keep `channel0_0` small and the decoded output
  readable.
- This is unrelated to `flash.bin` (native_sim's flash-simulator backing
  file, from `CONFIG_FLASH_SIMULATOR=y`) — don't confuse the two when
  checking whether tracing actually ran.
