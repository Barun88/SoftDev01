"""
Advanced File Operations Utility
Usage: util fileops [find|duplicate|backup|hash] [args]
- find <pattern>: Find files by pattern
- duplicate [dir]: Find duplicate files
- backup <source> <dest>: Backup files
- hash <file>: Calculate file hashes
"""

import sys
import os
import hashlib
import shutil
import glob
import time
from collections import defaultdict
from datetime import datetime, timedelta
import fnmatch

def get_size(bytes, suffix="B"):
    """Convert bytes to human readable format"""
    factor = 1024
    for unit in ["", "K", "M", "G", "T", "P"]:
        if bytes < factor:
            return f"{bytes:.2f}{unit}{suffix}"
        bytes /= factor

def calculate_hash(filepath, algorithm='md5'):
    """Calculate hash of a file"""
    hash_obj = hashlib.new(algorithm)
    try:
        with open(filepath, 'rb') as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_obj.update(chunk)
        return hash_obj.hexdigest()
    except (IOError, OSError):
        return None

def find_files(pattern, directory='.'):
    """Find files matching pattern"""
    print(f"Searching for files matching '{pattern}' in {os.path.abspath(directory)}")
    print("=" * 70)
    
    matches = []
    total_size = 0
    
    for root, dirs, files in os.walk(directory):
        for file in files:
            if fnmatch.fnmatch(file.lower(), pattern.lower()):
                filepath = os.path.join(root, file)
                try:
                    size = os.path.getsize(filepath)
                    modified = datetime.fromtimestamp(os.path.getmtime(filepath))
                    matches.append((filepath, size, modified))
                    total_size += size
                except (OSError, IOError):
                    continue
    
    if not matches:
        print(f"No files found matching '{pattern}'")
        return
    
    # Sort by size (largest first)
    matches.sort(key=lambda x: x[1], reverse=True)
    
    print(f"{'File Path':<50} {'Size':<10} {'Modified'}")
    print("-" * 70)
    
    for filepath, size, modified in matches:
        rel_path = os.path.relpath(filepath)[:49]
        size_str = get_size(size)
        mod_str = modified.strftime('%Y-%m-%d %H:%M')
        print(f"{rel_path:<50} {size_str:<10} {mod_str}")
    
    print(f"\nFound {len(matches)} files, Total size: {get_size(total_size)}")

def find_duplicates(directory='.'):
    """Find duplicate files"""
    print(f"Scanning for duplicate files in {os.path.abspath(directory)}")
    print("=" * 60)
    
    size_groups = defaultdict(list)
    hash_groups = defaultdict(list)
    
    # Group files by size first
    print("Analyzing file sizes...")
    for root, dirs, files in os.walk(directory):
        for file in files:
            filepath = os.path.join(root, file)
            try:
                size = os.path.getsize(filepath)
                if size > 0:  # Skip empty files
                    size_groups[size].append(filepath)
            except (OSError, IOError):
                continue
    
    # Find potential duplicates (same size)
    potential_duplicates = []
    for size, files in size_groups.items():
        if len(files) > 1:
            potential_duplicates.extend(files)
    
    if not potential_duplicates:
        print("No potential duplicates found (no files with same size)")
        return
    
    print(f"Found {len(potential_duplicates)} files with matching sizes")
    print("Calculating hashes...")
    
    # Calculate hashes for potential duplicates
    for filepath in potential_duplicates:
        file_hash = calculate_hash(filepath)
        if file_hash:
            hash_groups[file_hash].append(filepath)
    
    # Find actual duplicates
    duplicates_found = False
    total_wasted_space = 0
    
    for file_hash, files in hash_groups.items():
        if len(files) > 1:
            duplicates_found = True
            file_size = os.path.getsize(files[0])
            wasted_space = file_size * (len(files) - 1)
            total_wasted_space += wasted_space
            
            print(f"\nDuplicate group (Hash: {file_hash[:8]}...):")
            print(f"Size: {get_size(file_size)} each, Wasted space: {get_size(wasted_space)}")
            for i, filepath in enumerate(files):
                marker = "[ORIGINAL]" if i == 0 else "[DUPLICATE]"
                print(f"  {marker} {os.path.relpath(filepath)}")
    
    if duplicates_found:
        print(f"\nTotal wasted space: {get_size(total_wasted_space)}")
    else:
        print("No duplicate files found!")

def analyze_disk_usage(directory='.'):
    """Analyze disk usage"""
    print(f"Analyzing disk usage in {os.path.abspath(directory)}")
    print("=" * 60)
    
    dir_sizes = {}
    file_types = defaultdict(lambda: {'count': 0, 'size': 0})
    large_files = []
    
    total_size = 0
    total_files = 0
    
    for root, dirs, files in os.walk(directory):
        dir_size = 0
        
        for file in files:
            filepath = os.path.join(root, file)
            try:
                size = os.path.getsize(filepath)
                dir_size += size
                total_size += size
                total_files += 1
                
                # Track file types
                ext = os.path.splitext(file)[1].lower()
                if not ext:
                    ext = '[no extension]'
                file_types[ext]['count'] += 1
                file_types[ext]['size'] += size
                
                # Track large files (>10MB)
                if size > 10 * 1024 * 1024:
                    large_files.append((filepath, size))
                    
            except (OSError, IOError):
                continue
        
        if dir_size > 0:
            dir_sizes[root] = dir_size
    
    # Display directory sizes
    print("Largest Directories:")
    sorted_dirs = sorted(dir_sizes.items(), key=lambda x: x[1], reverse=True)[:10]
    for directory, size in sorted_dirs:
        rel_dir = os.path.relpath(directory)
        print(f"  {get_size(size):<10} {rel_dir}")
    
    # Display file type statistics
    print(f"\nFile Type Statistics:")
    sorted_types = sorted(file_types.items(), key=lambda x: x[1]['size'], reverse=True)[:10]
    for ext, info in sorted_types:
        print(f"  {ext:<15} {info['count']:>6} files  {get_size(info['size'])}")
    
    # Display large files
    if large_files:
        print(f"\nLarge Files (>10MB):")
        large_files.sort(key=lambda x: x[1], reverse=True)
        for filepath, size in large_files[:10]:
            rel_path = os.path.relpath(filepath)
            print(f"  {get_size(size):<10} {rel_path}")
    
    print(f"\nSummary:")
    print(f"  Total files: {total_files:,}")
    print(f"  Total size: {get_size(total_size)}")

def clean_temp_files(directory='.'):
    """Clean temporary files"""
    print(f"Cleaning temporary files in {os.path.abspath(directory)}")
    print("=" * 50)
    
    temp_patterns = [
        '*.tmp', '*.temp', '*.bak', '*.old', '*.orig',
        '*.log', '*.cache', '*~', '.DS_Store', 'Thumbs.db',
        '*.pyc', '*.pyo', '__pycache__'
    ]
    
    cleaned_files = []
    cleaned_size = 0
    
    for root, dirs, files in os.walk(directory):
        # Clean __pycache__ directories
        if '__pycache__' in dirs:
            pycache_path = os.path.join(root, '__pycache__')
            try:
                shutil.rmtree(pycache_path)
                print(f"Removed directory: {os.path.relpath(pycache_path)}")
                dirs.remove('__pycache__')  # Don't recurse into removed directory
            except (OSError, IOError) as e:
                print(f"Error removing {pycache_path}: {e}")
        
        for file in files:
            filepath = os.path.join(root, file)
            
            # Check if file matches any temp pattern
            should_clean = False
            for pattern in temp_patterns:
                if fnmatch.fnmatch(file.lower(), pattern.lower()):
                    should_clean = True
                    break
            
            # Check if file is old log file
            if file.endswith('.log'):
                try:
                    mtime = os.path.getmtime(filepath)
                    if time.time() - mtime > 7 * 24 * 3600:  # Older than 7 days
                        should_clean = True
                except (OSError, IOError):
                    pass
            
            if should_clean:
                try:
                    size = os.path.getsize(filepath)
                    os.remove(filepath)
                    cleaned_files.append(os.path.relpath(filepath))
                    cleaned_size += size
                    print(f"Removed: {os.path.relpath(filepath)} ({get_size(size)})")
                except (OSError, IOError) as e:
                    print(f"Error removing {filepath}: {e}")
    
    print(f"\nCleaning completed:")
    print(f"  Files removed: {len(cleaned_files)}")
    print(f"  Space freed: {get_size(cleaned_size)}")

def backup_files(source, destination):
    """Backup files from source to destination"""
    if not os.path.exists(source):
        print(f"Error: Source path '{source}' does not exist")
        return
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_name = f"backup_{timestamp}"
    backup_path = os.path.join(destination, backup_name)
    
    print(f"Creating backup...")
    print(f"Source: {os.path.abspath(source)}")
    print(f"Destination: {os.path.abspath(backup_path)}")
    print("=" * 50)
    
    try:
        if os.path.isfile(source):
            # Backup single file
            os.makedirs(destination, exist_ok=True)
            filename = os.path.basename(source)
            dest_file = os.path.join(backup_path, filename)
            os.makedirs(backup_path)
            shutil.copy2(source, dest_file)
            size = os.path.getsize(source)
            print(f"Backed up file: {filename} ({get_size(size)})")
        else:
            # Backup directory
            def copy_progress(src, dst):
                shutil.copy2(src, dst)
                size = os.path.getsize(src)
                rel_path = os.path.relpath(src, source)
                print(f"Copied: {rel_path} ({get_size(size)})")
            
            shutil.copytree(source, backup_path, copy_function=copy_progress)
        
        # Calculate total backup size
        total_size = 0
        if os.path.isfile(backup_path):
            total_size = os.path.getsize(backup_path)
        else:
            for root, dirs, files in os.walk(backup_path):
                for file in files:
                    filepath = os.path.join(root, file)
                    try:
                        total_size += os.path.getsize(filepath)
                    except (OSError, IOError):
                        continue
        
        print(f"\nBackup completed successfully!")
        print(f"Total backup size: {get_size(total_size)}")
        print(f"Backup location: {backup_path}")
        
    except Exception as e:
        print(f"Backup failed: {e}")

def calculate_file_hash(filepath):
    """Calculate and display file hashes"""
    if not os.path.isfile(filepath):
        print(f"Error: '{filepath}' is not a valid file")
        return
    
    print(f"Calculating hashes for: {os.path.abspath(filepath)}")
    print("=" * 60)
    
    algorithms = ['md5', 'sha1', 'sha256']
    file_size = os.path.getsize(filepath)
    
    print(f"File size: {get_size(file_size)}")
    print()
    
    for algorithm in algorithms:
        print(f"Calculating {algorithm.upper()}...")
        file_hash = calculate_hash(filepath, algorithm)
        if file_hash:
            print(f"{algorithm.upper()}: {file_hash}")
        else:
            print(f"{algorithm.upper()}: Error calculating hash")

def main():
    if len(sys.argv) < 2:
        print("Usage: util fileops [find|duplicate|size|clean|backup|hash] [args]")
        print("  find <pattern>      - Find files by pattern")
        print("  duplicate [dir]     - Find duplicate files")
        print("  size [dir]          - Analyze disk usage")
        print("  clean [dir]         - Clean temporary files")
        print("  backup <src> <dst>  - Backup files")
        print("  hash <file>         - Calculate file hashes")
        return
    
    command = sys.argv[1].lower()
    
    if command == "find":
        if len(sys.argv) < 3:
            print("Usage: util fileops find <pattern>")
        else:
            pattern = sys.argv[2]
            directory = sys.argv[3] if len(sys.argv) > 3 else '.'
            find_files(pattern, directory)
    
    elif command == "duplicate":
        directory = sys.argv[2] if len(sys.argv) > 2 else '.'
        find_duplicates(directory)
    
    elif command == "size":
        directory = sys.argv[2] if len(sys.argv) > 2 else '.'
        analyze_disk_usage(directory)
    
    elif command == "clean":
        directory = sys.argv[2] if len(sys.argv) > 2 else '.'
        clean_temp_files(directory)
    
    elif command == "backup":
        if len(sys.argv) < 4:
            print("Usage: util fileops backup <source> <destination>")
        else:
            backup_files(sys.argv[2], sys.argv[3])
    
    elif command == "hash":
        if len(sys.argv) < 3:
            print("Usage: util fileops hash <file>")
        else:
            calculate_file_hash(sys.argv[2])
    
    else:
        print("Unknown command. Use: find, duplicate, size, clean, backup, or hash")

if __name__ == "__main__":
    main()