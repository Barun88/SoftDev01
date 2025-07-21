#!/usr/bin/env python3
"""
System Information Utility
Usage: util sysinfo [option]
Options: cpu, memory, disk, network, all
"""

import sys
import platform
import psutil
import socket
from datetime import datetime

def get_size(bytes, suffix="B"):
    """Convert bytes to human readable format"""
    factor = 1024
    for unit in ["", "K", "M", "G", "T", "P"]:
        if bytes < factor:
            return f"{bytes:.2f}{unit}{suffix}"
        bytes /= factor

def show_cpu_info():
    print("=" * 40)
    print("CPU Information")
    print("=" * 40)
    print(f"Processor: {platform.processor()}")
    print(f"Architecture: {platform.machine()}")
    print(f"Physical cores: {psutil.cpu_count(logical=False)}")
    print(f"Total cores: {psutil.cpu_count(logical=True)}")
    
    # CPU frequencies
    cpufreq = psutil.cpu_freq()
    print(f"Max Frequency: {cpufreq.max:.2f}Mhz")
    print(f"Min Frequency: {cpufreq.min:.2f}Mhz")
    print(f"Current Frequency: {cpufreq.current:.2f}Mhz")
    
    # CPU usage per core
    print("CPU Usage Per Core:")
    for i, percentage in enumerate(psutil.cpu_percent(percpu=True, interval=1)):
        print(f"  Core {i}: {percentage}%")
    print(f"Total CPU Usage: {psutil.cpu_percent()}%")

def show_memory_info():
    print("=" * 40)
    print("Memory Information")
    print("=" * 40)
    
    # Physical memory
    svmem = psutil.virtual_memory()
    print(f"Total: {get_size(svmem.total)}")
    print(f"Available: {get_size(svmem.available)}")
    print(f"Used: {get_size(svmem.used)}")
    print(f"Percentage: {svmem.percent}%")
    
    print("\nSWAP Memory:")
    swap = psutil.swap_memory()
    print(f"Total: {get_size(swap.total)}")
    print(f"Free: {get_size(swap.free)}")
    print(f"Used: {get_size(swap.used)}")
    print(f"Percentage: {swap.percent}%")

def show_disk_info():
    print("=" * 40)
    print("Disk Information")
    print("=" * 40)
    
    partitions = psutil.disk_partitions()
    for partition in partitions:
        print(f"Device: {partition.device}")
        print(f"  Mountpoint: {partition.mountpoint}")
        print(f"  File system: {partition.fstype}")
        
        try:
            partition_usage = psutil.disk_usage(partition.mountpoint)
            print(f"  Total Size: {get_size(partition_usage.total)}")
            print(f"  Used: {get_size(partition_usage.used)}")
            print(f"  Free: {get_size(partition_usage.free)}")
            print(f"  Percentage: {(partition_usage.used/partition_usage.total)*100:.1f}%")
        except PermissionError:
            print("  Permission Denied")
        print()
    
    # Disk I/O
    disk_io = psutil.disk_io_counters()
    print(f"Total read: {get_size(disk_io.read_bytes)}")
    print(f"Total write: {get_size(disk_io.write_bytes)}")

def show_network_info():
    print("=" * 40)
    print("Network Information")
    print("=" * 40)
    
    # Hostname and IP
    hostname = socket.gethostname()
    local_ip = socket.gethostbyname(hostname)
    print(f"Hostname: {hostname}")
    print(f"Local IP: {local_ip}")
    
    # Network interfaces
    if_addrs = psutil.net_if_addrs()
    for interface_name, interface_addresses in if_addrs.items():
        print(f"\nInterface: {interface_name}")
        for address in interface_addresses:
            if str(address.family) == 'AddressFamily.AF_INET':
                print(f"  IP Address: {address.address}")
                print(f"  Netmask: {address.netmask}")
                print(f"  Broadcast IP: {address.broadcast}")
            elif str(address.family) == 'AddressFamily.AF_PACKET':
                print(f"  MAC Address: {address.address}")
                print(f"  Netmask: {address.netmask}")
                print(f"  Broadcast MAC: {address.broadcast}")
    
    # Network I/O
    net_io = psutil.net_io_counters()
    print(f"\nTotal Bytes Sent: {get_size(net_io.bytes_sent)}")
    print(f"Total Bytes Received: {get_size(net_io.bytes_recv)}")

def show_system_info():
    print("=" * 40)
    print("System Information")
    print("=" * 40)
    print(f"System: {platform.system()}")
    print(f"Node Name: {platform.node()}")
    print(f"Release: {platform.release()}")
    print(f"Version: {platform.version()}")
    print(f"Machine: {platform.machine()}")
    print(f"Processor: {platform.processor()}")
    
    # Boot time
    boot_time_timestamp = psutil.boot_time()
    bt = datetime.fromtimestamp(boot_time_timestamp)
    print(f"Boot Time: {bt.year}/{bt.month}/{bt.day} {bt.hour}:{bt.minute}:{bt.second}")

def main():
    if len(sys.argv) < 2:
        option = "all"
    else:
        option = sys.argv[1].lower()
    
    if option in ["all", "system"]:
        show_system_info()
        print()
    
    if option in ["all", "cpu"]:
        show_cpu_info()
        print()
    
    if option in ["all", "memory", "mem"]:
        show_memory_info()
        print()
    
    if option in ["all", "disk"]:
        show_disk_info()
        print()
    
    if option in ["all", "network", "net"]:
        show_network_info()
        print()
    
    if option not in ["all", "cpu", "memory", "mem", "disk", "network", "net", "system"]:
        print("Usage: util sysinfo [option]")
        print("Options: cpu, memory, disk, network, system, all")

if __name__ == "__main__":
    main()