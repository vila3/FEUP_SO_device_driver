serp.o: /host/lab3/serp/serp.c include/linux/autoconf.h \
  include/linux/init.h include/linux/compiler.h \
  include/linux/compiler-gcc4.h include/linux/compiler-gcc.h \
  include/linux/module.h include/linux/sched.h include/linux/auxvec.h \
  include/asm/auxvec.h include/asm/param.h include/linux/capability.h \
  include/linux/types.h include/linux/posix_types.h \
  include/linux/stddef.h include/asm/posix_types.h include/asm/types.h \
  include/linux/spinlock.h include/linux/preempt.h \
  include/linux/thread_info.h include/linux/bitops.h include/asm/bitops.h \
  include/asm/alternative.h include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h include/asm-generic/bitops/minix.h \
  include/asm/thread_info.h include/asm/page.h \
  include/asm-generic/memory_model.h include/asm-generic/page.h \
  include/asm/processor.h include/asm/vm86.h include/asm/math_emu.h \
  include/asm/sigcontext.h include/asm/segment.h include/asm/cpufeature.h \
  include/asm/msr.h include/asm/system.h include/linux/kernel.h \
  /usr/lib/gcc/i486-linux-gnu/4.1.2/include/stdarg.h \
  include/linux/linkage.h include/asm/linkage.h include/asm/bug.h \
  include/asm-generic/bug.h include/linux/irqflags.h \
  include/asm/irqflags.h include/linux/cache.h include/asm/cache.h \
  include/linux/threads.h include/asm/percpu.h \
  include/asm-generic/percpu.h include/linux/cpumask.h \
  include/linux/bitmap.h include/linux/string.h include/asm/string.h \
  include/linux/stringify.h include/linux/bottom_half.h \
  include/linux/spinlock_types.h include/linux/lockdep.h \
  include/linux/spinlock_types_up.h include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h include/asm/atomic.h \
  include/asm-generic/atomic.h include/asm/current.h \
  include/linux/timex.h include/linux/time.h include/linux/seqlock.h \
  include/asm/timex.h include/asm/tsc.h include/linux/jiffies.h \
  include/linux/calc64.h include/asm/div64.h include/linux/rbtree.h \
  include/linux/errno.h include/asm/errno.h include/asm-generic/errno.h \
  include/asm-generic/errno-base.h include/linux/nodemask.h \
  include/linux/numa.h include/asm/semaphore.h include/linux/wait.h \
  include/linux/list.h include/linux/poison.h include/linux/prefetch.h \
  include/linux/rwsem.h include/asm/rwsem.h include/asm/ptrace.h \
  include/asm/ptrace-abi.h include/asm/mmu.h include/asm/cputime.h \
  include/asm-generic/cputime.h include/linux/smp.h include/linux/sem.h \
  include/linux/ipc.h include/asm/ipcbuf.h include/linux/kref.h \
  include/asm/sembuf.h include/linux/signal.h include/asm/signal.h \
  include/asm-generic/signal.h include/asm/siginfo.h \
  include/asm-generic/siginfo.h include/linux/securebits.h \
  include/linux/fs_struct.h include/linux/completion.h \
  include/linux/pid.h include/linux/rcupdate.h include/linux/percpu.h \
  include/linux/slab.h include/linux/gfp.h include/linux/mmzone.h \
  include/linux/memory_hotplug.h include/linux/notifier.h \
  include/linux/mutex.h include/linux/srcu.h include/linux/topology.h \
  include/asm/topology.h include/asm-generic/topology.h \
  include/linux/kmalloc_sizes.h include/linux/seccomp.h \
  include/linux/futex.h include/linux/rtmutex.h include/linux/plist.h \
  include/linux/param.h include/linux/resource.h include/asm/resource.h \
  include/asm-generic/resource.h include/linux/timer.h \
  include/linux/hrtimer.h include/linux/ktime.h include/linux/aio.h \
  include/linux/workqueue.h include/linux/aio_abi.h include/linux/uio.h \
  include/linux/sysdev.h include/linux/kobject.h include/linux/sysfs.h \
  include/linux/pm.h include/linux/stat.h include/asm/stat.h \
  include/linux/kmod.h include/linux/elf.h include/linux/elf-em.h \
  include/asm/elf.h include/asm/user.h include/linux/utsname.h \
  include/linux/nsproxy.h include/asm/desc.h include/asm/ldt.h \
  include/linux/moduleparam.h include/asm/local.h include/asm/module.h \
  include/linux/mm.h include/linux/prio_tree.h include/linux/fs.h \
  include/linux/limits.h include/linux/ioctl.h include/asm/ioctl.h \
  include/asm-generic/ioctl.h include/linux/kdev_t.h \
  include/linux/dcache.h include/linux/radix-tree.h include/linux/quota.h \
  include/linux/dqblk_xfs.h include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h include/linux/nfs_fs_i.h include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h include/linux/fcntl.h \
  include/asm/fcntl.h include/asm-generic/fcntl.h include/linux/err.h \
  include/linux/debug_locks.h include/linux/backing-dev.h \
  include/linux/mm_types.h include/asm/pgtable.h include/asm/fixmap.h \
  include/asm/acpi.h include/acpi/pdc_intel.h include/asm/apicdef.h \
  include/asm/kmap_types.h include/asm/pgtable-2level-defs.h \
  include/asm/pgtable-2level.h include/asm-generic/pgtable-nopmd.h \
  include/asm-generic/pgtable-nopud.h include/asm-generic/pgtable.h \
  include/linux/page-flags.h include/linux/vmstat.h include/asm/uaccess.h \
  include/asm/io.h include/asm-generic/iomap.h include/linux/vmalloc.h \
  include/linux/ioport.h include/linux/cdev.h include/linux/delay.h \
  include/asm/delay.h /host/lab3/serp/serial_reg.h
