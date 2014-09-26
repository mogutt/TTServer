package com.mogujie.ares.extend.action;

import java.lang.management.ManagementFactory;

import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.net.DataBuffer;
import com.sun.management.OperatingSystemMXBean;

@SuppressWarnings("restriction")
public class Dashboard extends BaseAction {

    private PerformanceMonitor cpuMonitor;

    public Dashboard() {
        // Create CPU monitoring class
        cpuMonitor = new PerformanceMonitor();
        cpuMonitor.getCpuUsage(); // Set baseline
    }

    public DataBuffer getServerStatus(int version) {

        DataBuffer buffer = new DataBuffer();

        // system info
        double cpu = roundToDecimals(cpuMonitor.getCpuUsage() * 100, 2);
        long freeMem = Runtime.getRuntime().freeMemory();
        long maxMem = Runtime.getRuntime().maxMemory();
        long totalMem = Runtime.getRuntime().totalMemory();

        buffer.writeDouble(cpu);
        buffer.writeLong(freeMem);
        buffer.writeLong(maxMem);
        buffer.writeLong(totalMem);

        // threads list
        long totalThreadsCpuTime = 0;
        long[] threadIds = ManagementFactory.getThreadMXBean()
                .getAllThreadIds();

        buffer.writeInt(threadIds.length); // 多少条线程.
        for (int i = 0; i < threadIds.length; i++) {
            long id = threadIds[i];
            String name = ManagementFactory.getThreadMXBean().getThreadInfo(id)
                    .getThreadName();
            long cpuTime = 0;
            if (ManagementFactory.getThreadMXBean().isThreadCpuTimeSupported()
                    && ManagementFactory.getThreadMXBean()
                            .isThreadCpuTimeEnabled()) {
                cpuTime = ManagementFactory.getThreadMXBean().getThreadCpuTime(
                        id);
                totalThreadsCpuTime += cpuTime;
            }
            buffer.writeLong(id);
            buffer.writeString(name);
            buffer.writeLong(cpuTime);
        }

        buffer.writeLong(totalThreadsCpuTime);

        return buffer;
    }

    private static double roundToDecimals(double d, int c) {
        int temp = (int) ((d * Math.pow(10, c)));
        return (double) (temp / Math.pow(10, c));
    }

    private class PerformanceMonitor {
        private long lastSystemTime = 0;
        private long lastProcessCpuTime = 0;
        OperatingSystemMXBean osMxBean = (OperatingSystemMXBean) ManagementFactory
                .getOperatingSystemMXBean();

        public synchronized double getCpuUsage() {
            if (lastSystemTime == 0) {
                baselineCounters();
                return 0;
            }

            long systemTime = System.nanoTime();
            long processCpuTime = osMxBean.getProcessCpuTime();

            double cpuUsage = (double) (processCpuTime - lastProcessCpuTime)
                    / (systemTime - lastSystemTime);

            lastSystemTime = systemTime;
            lastProcessCpuTime = processCpuTime;

            return cpuUsage / osMxBean.getAvailableProcessors();
        }

        private void baselineCounters() {
            lastSystemTime = System.nanoTime();
            lastProcessCpuTime = osMxBean.getProcessCpuTime();
        }
    }
}
