"""
Process Manager Utility
Usage: util procman [list|kill|info|top|search] [args]
- list: List all running processes
- kill <pid>: Kill process by PID
- info <pid>: Show detailed process information
- top: Show top processes by CPU usage
- search <name>: Search for processes by name
"""

import sys
import psutil
import time
from datetime import datetime

def get_size(bytes, suffix="B"):
    """Convert bytes to human readable format"""
    factor = 1024
    for unit in ["", "K", "M", "G", "T", "P"]:
        if bytes < factor:
            return f"{bytes:.2f}{unit}{suffix}"
        bytes /= factor

def list_processes():
    print("=" * 80)
    print("Running Processes")
    print("=" * 80)
    print(f"{'PID':<8} {'Name':<20} {'CPU%':<8} {'Memory':<10} {'Status':<12} {'User'}")
    print("-" * 80)
    
    processes = []
    for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_info', 'status', 'username']):
        try:
            processes.append(proc.info)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
    
    # Sort by CPU usage
    processes.sort(key=lambda x: x['cpu_percent'] or 0, reverse=True)
    
    for proc in processes[:50]:  # Show top 50 processes
        pid = proc['pid']
        name = proc['name'][:19] if proc['name'] else 'N/A'
        cpu = f"{proc['cpu_percent']:.1f}" if proc['cpu_percent'] else '0.0'
        memory = get_size(proc['memory_info'].rss) if proc['memory_info'] else 'N/A'
        status = proc['status'][:11] if proc['status'] else 'N/A'
        user = proc['username'][:15] if proc['username'] else 'N/A'
        
        print(f"{pid:<8} {name:<20} {cpu:<8} {memory:<10} {status:<12} {user}")

def kill_process(pid):
    try:
        pid = int(pid)
        process = psutil.Process(pid)
        process_name = process.name()
        
        print(f"Attempting to kill process: {process_name} (PID: {pid})")
        process.terminate()
        
        # Wait for process to terminate
        try:
            process.wait(timeout=5)
            print(f"Process {process_name} (PID: {pid}) terminated successfully.")
        except psutil.TimeoutExpired:
            print(f"Process didn't terminate gracefully, force killing...")
            process.kill()
            print(f"Process {process_name} (PID: {pid}) force killed.")
            
    except ValueError:
        print("Error: Invalid PID. Please provide a numeric PID.")
    except psutil.NoSuchProcess:
        print(f"Error: No process found with PID {pid}")
    except psutil.AccessDenied:
        print(f"Error: Access denied. Cannot kill process {pid}")
    except Exception as e:
        print(f"Error: {e}")

def show_process_info(pid):
    try:
        pid = int(pid)
        process = psutil.Process(pid)
        
        print("=" * 50)
        print(f"Process Information - PID: {pid}")
        print("=" * 50)
        
        # Basic info
        print(f"Name: {process.name()}")
        print(f"PID: {process.pid}")
        print(f"PPID: {process.ppid()}")
        print(f"Status: {process.status()}")
        print(f"Username: {process.username()}")
        
        # CPU and Memory
        print(f"CPU Percent: {process.cpu_percent()}%")
        memory_info = process.memory_info()
        print(f"Memory RSS: {get_size(memory_info.rss)}")
        print(f"Memory VMS: {get_size(memory_info.vms)}")
        print(f"Memory Percent: {process.memory_percent():.2f}%")
        
        # Process times
        create_time = datetime.fromtimestamp(process.create_time())
        print(f"Created: {create_time}")
        
        # Command line
        try:
            cmdline = ' '.join(process.cmdline())
            if cmdline:
                print(f"Command Line: {cmdline[:100]}...")
        except (psutil.AccessDenied, psutil.ZombieProcess):
            print("Command Line: Access Denied")
        
        # Working directory
        try:
            print(f"Working Directory: {process.cwd()}")
        except (psutil.AccessDenied, psutil.NoSuchProcess):
            print("Working Directory: Access Denied")
        
        # Open files
        try:
            open_files = process.open_files()
            if open_files:
                print(f"Open Files ({len(open_files)}):")
                for file in open_files[:5]:  # Show first 5
                    print(f"  {file.path}")
                if len(open_files) > 5:
                    print(f"  ... and {len(open_files) - 5} more")
        except (psutil.AccessDenied, psutil.NoSuchProcess):
            print("Open Files: Access Denied")
        
        # Network connections
        try:
            connections = process.connections()
            if connections:
                print(f"Network Connections ({len(connections)}):")
                for conn in connections[:3]:  # Show first 3
                    local = f"{conn.laddr.ip}:{conn.laddr.port}" if conn.laddr else "N/A"
                    remote = f"{conn.raddr.ip}:{conn.raddr.port}" if conn.raddr else "N/A"
                    print(f"  {local} -> {remote} ({conn.status})")
        except (psutil.AccessDenied, psutil.NoSuchProcess):
            print("Network Connections: Access Denied")
            
    except ValueError:
        print("Error: Invalid PID. Please provide a numeric PID.")
    except psutil.NoSuchProcess:
        print(f"Error: No process found with PID {pid}")
    except Exception as e:
        print(f"Error: {e}")

def show_top_processes():
    print("=" * 80)
    print("Top Processes by CPU Usage")
    print("=" * 80)
    print(f"{'PID':<8} {'Name':<20} {'CPU%':<8} {'Memory%':<10} {'Memory':<10} {'Threads'}")
    print("-" * 80)
    
    processes = []
    for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_percent', 'memory_info', 'num_threads']):
        try:
            proc.info['cpu_percent'] = proc.cpu_percent()
            processes.append(proc.info)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
    
    # Sort by CPU usage
    processes.sort(key=lambda x: x['cpu_percent'] or 0, reverse=True)
    
    for proc in processes[:20]:  # Show top 20
        pid = proc['pid']
        name = proc['name'][:19] if proc['name'] else 'N/A'
        cpu = f"{proc['cpu_percent']:.1f}" if proc['cpu_percent'] else '0.0'
        mem_percent = f"{proc['memory_percent']:.1f}" if proc['memory_percent'] else '0.0'
        memory = get_size(proc['memory_info'].rss) if proc['memory_info'] else 'N/A'
        threads = proc['num_threads'] if proc['num_threads'] else 'N/A'
        
        print(f"{pid:<8} {name:<20} {cpu:<8} {mem_percent:<10} {memory:<10} {threads}")

def search_processes(search_term):
    print(f"Searching for processes containing: '{search_term}'")
    print("=" * 60)
    print(f"{'PID':<8} {'Name':<25} {'CPU%':<8} {'Memory':<10} {'Status'}")
    print("-" * 60)
    
    found = False
    for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_info', 'status']):
        try:
            if search_term.lower() in proc.info['name'].lower():
                found = True
                pid = proc.info['pid']
                name = proc.info['name'][:24]
                cpu = f"{proc.info['cpu_percent'] or 0:.1f}"
                memory = get_size(proc.info['memory_info'].rss) if proc.info['memory_info'] else 'N/A'
                status = proc.info['status'][:10] if proc.info['status'] else 'N/A'
                
                print(f"{pid:<8} {name:<25} {cpu:<8} {memory:<10} {status}")
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
    
    if not found:
        print(f"No processes found containing '{search_term}'")

def show_system_summary():
    print("=" * 50)
    print("System Process Summary")
    print("=" * 50)
    
    # Count processes by status
    status_count = {}
    total_processes = 0
    
    for proc in psutil.process_iter(['status']):
        try:
            status = proc.info['status']
            status_count[status] = status_count.get(status, 0) + 1
            total_processes += 1
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
    
    print(f"Total Processes: {total_processes}")
    print("\nProcess Status Distribution:")
    for status, count in status_count.items():
        print(f"  {status}: {count}")
    
    # CPU and Memory summary
    cpu_percent = psutil.cpu_percent(interval=1)
    memory = psutil.virtual_memory()
    
    print(f"\nSystem Resources:")
    print(f"  CPU Usage: {cpu_percent}%")
    print(f"  Memory Usage: {memory.percent}%")
    print(f"  Available Memory: {get_size(memory.available)}")

def main():
    if len(sys.argv) < 2:
        show_system_summary()
        print()
        list_processes()
        return
    
    command = sys.argv[1].lower()
    
    if command == "list":
        list_processes()
    elif command == "kill":
        if len(sys.argv) < 3:
            print("Usage: util procman kill <pid>")
        else:
            kill_process(sys.argv[2])
    elif command == "info":
        if len(sys.argv) < 3:
            print("Usage: util procman info <pid>")
        else:
            show_process_info(sys.argv[2])
    elif command == "top":
        show_top_processes()
    elif command == "search":
        if len(sys.argv) < 3:
            print("Usage: util procman search <process_name>")
        else:
            search_processes(sys.argv[2])
    else:
        print("Usage: util procman [list|kill|info|top|search] [args]")
        print("  list           - List all running processes")
        print("  kill <pid>     - Kill process by PID")
        print("  info <pid>     - Show detailed process information")
        print("  top            - Show top processes by CPU usage")
        print("  search <name>  - Search for processes by name")

if __name__ == "__main__":
    main()