import os
import subprocess
import sys


def compile_and_compare(cpp_file):
  # determine executable name
  if os.name == 'nt':  
    exe_file = os.path.splitext(cpp_file)[0] + '.exe'

  # 1. Compile file
  print(f'Compiling {cpp_file}...')
  compile_proc = subprocess.run(
      ['g++', '-std=c++17', '-fopenmp', '-O2', '-Wall', '-Wextra', cpp_file, '-o', exe_file], capture_output=True, text=True
  )

  if compile_proc.returncode != 0:
    print('Compilation failed:')
    print(compile_proc.stderr)
    exit("ERROR: COMPILE FAILURE!")
    return

  # 2. Run the executable twice
  print('Running program first time...')
  run1 = subprocess.run([exe_file, "12345"], timeout=30, capture_output=True, text=True)

  print('Running program second time...')
  run2 = subprocess.run([exe_file, "12345"], timeout=30, capture_output=True, text=True)

  print('Running program third time...')
  run3 = subprocess.run([exe_file], timeout=30, capture_output=True, text=True)

  # 2 a. Error catching
  if run1.returncode != 0:
    exit("Error ", run1.returncode, ": Issue with Run 1.")
  if run2.returncode != 0:
    exit("Error ", run2.returncode, ": Issue with Run 2.")
  if run3.returncode != 0:
    exit("Error ", run3.returncode, ": Issue with Run 3.")

  # 3. Compare outputs
  out1 = run1.stdout.strip()
  out2 = run2.stdout.strip()
  out3 = run3.stdout.strip()

  if out1 == "":
      exit("Error: No Output from Run 1.")
  if out2 == "":
      exit("Error: No Output from Run 2.")
  if out3 == "":
       exit("Error: No Output from Run 3.")

  if out1 == "":
      exit("Error: No Output from Run 1.")

  # 4. Print results

  print('\n--- Result ---')
  if out1 == out2:
    print('Outputs 1 & 2 are IDENTICAL (as expected).')
  else:
    print('Outputs 1 & 2 are DIFFERENT.')
    print('\n--- Output 1 ---')
    print(out1)
    print('\n--- Output 2 ---')
    print(out2)
  if out1 != out3 != out2:
    print('Output 3 is DIFFERENT (as expected.)')
  else:
      print('Output 3 is the same.')


if __name__ == '__main__':
  if len(sys.argv) < 2:
    print('Usage: python field_sim_test.py <path_to_file.cpp>')
    sys.exit(1)

  cpp_path = sys.argv[1]
  compile_and_compare(cpp_path)