# ESP32-S3 Memory Quick Reference

## Memory Usage Example
```
Memory region         Used Size  Region Size  %age Used
           FLASH:     2677716 B   16776960 B     15.96%
     iram0_0_seg:       64560 B     343552 B     18.79%
     dram0_0_seg:      260048 B     327168 B     79.48%
     irom0_0_seg:     1433106 B        32 MB      4.27%
     drom0_0_seg:     2481108 B        32 MB      7.39%
    ext_dram_seg:     8167568 B         8 MB     97.36%
    ext_iram_seg:          0 GB         8 MB      0.00%
    rtc_iram_seg:          0 GB         8 KB      0.00%
    rtc_slow_seg:          0 GB         8 KB      0.00%
        IDT_LIST:          0 GB         8 KB      0.00%
```

## Memory Regions

### FLASH (External Flash Chip)
- **What**: Physical storage for your compiled binary
- **Contains**: All sections (irom, drom, iram, dram) before they're loaded
- **Note**: Total FLASH ≠ irom + drom because it also includes code/data that gets copied to IRAM/DRAM at boot

### iram0_0_seg (Instruction RAM - Internal)
- **Size**: ~335 KB
- **Speed**: 2-3× faster than flash
- **Use for**: ISRs, interrupt handlers, time-critical code, WiFi/BT code
- **Place code here**: `void IRAM_ATTR my_function(void) { }`

### dram0_0_seg (Data RAM - Internal)
- **Size**: ~319 KB  
- **Use for**: Stacks, DMA buffers, globals, heap (when no SPIRAM)
- **Critical**: Required for DMA and stack space
- **Place data here**: `DRAM_ATTR uint8_t buffer[1024];`

### irom0_0_seg (Instruction ROM - Virtual/Cached)
- **Size**: 32 MB virtual space
- **What**: Code executes directly from flash via cache (XIP)
- **Speed**: ~70-80% of IRAM with warm cache
- **Use for**: Most application code (default)
- **Default**: Regular functions go here automatically

### drom0_0_seg (Data ROM - Virtual/Cached)  
- **Size**: 32 MB virtual space
- **What**: Read-only data mapped from flash
- **Use for**: Const strings, lookup tables, fonts, images
- **Default**: `const` data goes here automatically

### ext_dram_seg (External SPIRAM)
- **Size**: Up to 32 MB (typically 8 MB)
- **Speed**: 3-4× slower than internal DRAM
- **Use for**: Large buffers, LVGL framebuffers, heap allocations
- **Cannot use for**: DMA buffers, stacks, time-critical data
- **Place data here**: `EXT_RAM_BSS_ATTR uint8_t big_buffer[1MB];`

**Important**: SPIRAM shares virtual address space with drom0. Usable SPIRAM = Total - drom0_size (rounded to 64KB pages)

### rtc_iram_seg (RTC Fast RAM)
- **Size**: 8 KB
- **Special**: Retains data during deep sleep, executable
- **Use for**: Wake stub code, persistent counters
- **Usage**: `RTC_FAST_ATTR uint32_t wake_count;`

### rtc_slow_seg (RTC Slow RAM)
- **Size**: 8 KB
- **Special**: Retains data during deep sleep, lower power, not executable
- **Use for**: Persistent data, ULP programs
- **Usage**: `RTC_DATA_ATTR uint32_t boot_count;`

## Key Architecture Points

### IRAM/DRAM Share Physical Memory
- Same physical SRAM1 (416 KB), accessed via different buses
- IRAM: Instruction bus (Ibus) at 0x4037xxxx
- DRAM: Data bus (Dbus) at 0x3fcxxxxx
- Linker ensures they don't overlap

### Load (LMA) vs Runtime (VMA) Addresses
- **IRAM/DRAM**: Copied from flash (LMA) to SRAM (VMA) at boot
- **IROM/DROM**: Mapped from flash (LMA) to virtual addresses (VMA) by MMU
- **SPIRAM**: Direct runtime allocation (VMA only)

## Quick Decision Guide

| Need | Use |
|------|-----|
| ISR or critical timing | iram0 (IRAM_ATTR) |
| DMA buffer | dram0 (DRAM_ATTR) |
| Stack space | dram0 (automatic) |
| Regular code | irom0 (default) |
| Const data | drom0 (automatic) |
| Large buffers | ext_dram (malloc or EXT_RAM_BSS_ATTR) |
| Deep sleep data | rtc RAM (RTC_DATA_ATTR) |
