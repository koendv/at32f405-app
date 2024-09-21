# memwatch - read memory while target running

This PR introduces a new monitor command, *memwatch*.
The memwatch command reads target memory while the target is running.
Inspecting memory without halting the target is useful when debugging hard real time systems.

The arguments to "mon memwatch" are names, formats, and memory addresses. Up to 8 memory addresses may be monitored at the same time. The "mon memwatch" command can be applied to Arm Cortex-M targets only.

## Example

Assume target firmware contains a variable called "counter". In gdb:

```
(gdb) p &counter
$1 = (<data variable, no debug info> *) 0x20000224 <counter>
(gdb) mon memwatch counter /d 0x20000224
0x20000224
(gdb) r
```
When the variable changes, output is written to the usb serial:

```
counter 0
counter 1
counter 2
counter 3
counter 4
```
To switch off, enter "mon memwatch" without arguments.

```
(gdb) mon memwatch
```

## Arguments

The command arguments in detail:

```
mon memwatch [/t] [[NAME] [/d|/u|/f|/x] ADDRESS]...
```

Up to 8 addresses of watchpoints may be specified.

```
mon memwatch x1 /d 0x20000224 x2 0x20000228 x3 0x2000022c x4 0x20000230 x5 0x20000234 x6 0x20000238 x7 0x2000023c x8 0x20000240
```

NAME begins with a letter a-z A-Z. NAME is printed on every line of output. If NAME has been omitted, the address is printed instead:

```
(gdb) mon memwatch /d 0x20000224
```
prints
```
0x20000224 0
0x20000224 1
0x20000224 2
0x20000224 3
0x20000224 4
```

Format is one of

- /d signed 32-bit integer
- /u unsigned 32-bit integer
- /f 32-bit float
- /x hex 32-bit integer
- /t timestamp in ms

If format is not given, default is hex, or the last used format.

```
(gdb) mon memwatch counter 0x20000224
```
prints
```
counter 0x0
counter 0x1
counter 0x2
counter 0x3
counter 0x4
```

Address is a hexadecimal number. The address is the minimum necessary to define a watchpoint.

```
(gdb) mon memwatch 0x20000224
```
prints
```
0x20000224 0x0
0x20000224 0x1
0x20000224 0x2
0x20000224 0x3
0x20000224 0x4
```
The '/f' format prints IEEE754 32-bit single-precision "float":
```
(gdb) mon memwatch counter /d 0x20000224 root /f 0x20000248
```
prints
```
counter 0
root 0
counter 1
root 1
counter 2
root 1.41421
counter 3
root 1.73205
counter 4
root 2
```

On the Black Magic Probe firmware probes /f defaults to printing all significant digits, without any rounding.
On the pc-based Black Magic Debug App /f defaults to printing to 6 decimal places "%.6g".
Optionally, /f may be followed by a digit, indicating the number of decimal places.
Example: /f4 prints floating point numbers to four decimal places.

The '/t' format prints a timestamp in milliseconds:
```
(gdb) mon memwatch /t counter /d 0x20000224
```
prints
```
20601 counter 0
20613 counter 1
21613 counter 2
22613 counter 3
23613 counter 4
```

With both target and bmp running on a stm32f411, memory is polled 1000 times per second.
