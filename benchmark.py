import subprocess
import re
import json
import os
import argparse
from typing import Dict, List, Optional

class BenchmarkRunner:
    def __init__(self, executable_path: str = "./build/a"):
        self.executable_path = executable_path

    def run_config(self, n: int, m: int, threads: int) -> Dict:
        """Runs the solver with given configuration and returns parsed results."""
        cmd = [self.executable_path, str(n), str(m), str(threads)]
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            output = result.stdout
            
            # Parsing logic
            metrics_match = re.search(r"Error:\s+([\d.e+-]+)\s+;\s+Residual:\s+([\d.e+-]+)\s+\(([\d.e+-]+)\)", output)
            time_match = re.search(r"Total time in seconds:\s+([\d.]+)", output)

            res = {
                "n": n,
                "m": m,
                "threads": threads,
                "success": True,
                "error": float(metrics_match.group(1)) if metrics_match else None,
                "residual": float(metrics_match.group(2)) if metrics_match else None,
                "residual_rel": float(metrics_match.group(3)) if metrics_match else None,
            }
            
            if time_match:
                res["time_s"] = float(time_match.group(1))
            
            return res
        except subprocess.CalledProcessError as e:
            return {
                "n": n,
                "m": m,
                "threads": threads,
                "success": False,
                "exit_code": e.returncode,
                "stderr": e.stderr
            }

def run_suite(runner: BenchmarkRunner, configs: List[Dict]) -> List[Dict]:
    results = []
    for conf in configs:
        print(f"Running N={conf['n']}, M={conf['m']}, Threads={conf['threads']}... ", end="", flush=True)
        res = runner.run_config(conf['n'], conf['m'], conf['threads'])
        if res['success']:
            print(f"Done. Time: {res.get('time_s', 0):.2f}s, Error: {res['error']:.2e}")
        else:
            print(f"FAILED. Exit code: {res.get('exit_code')}")
        results.append(res)
    return results

def compare_results(baseline: List[Dict], current: List[Dict], tolerance: float = 1e-12):
    print("\n--- Regression Report ---")
    all_pass = True
    for b, c in zip(baseline, current):
        if not c['success']:
            print(f"FAIL: Configuration N={c['n']} M={c['m']} T={c['threads']} failed to run.")
            all_pass = False
            continue
        
        err_diff = abs(b['error'] - c['error'])
        if err_diff > tolerance:
            print(f"REGRESSION: N={c['n']} M={c['m']} T={c['threads']} - Error diff: {err_diff:.2e} (Baseline: {b['error']:.2e}, Current: {c['error']:.2e})")
            all_pass = False
        else:
            time_ratio = c['time_s'] / b['time_s'] if b.get('time_s', 0) > 0 else 1.0
            status = "PASS"
            if time_ratio > 1.2: status = "PASS (SLOWER)"
            elif time_ratio < 0.8: status = "PASS (FASTER)"
            print(f"{status}: N={c['n']} M={c['m']} T={c['threads']} (Error: {c['error']:.2e}, Time: {c['time_s']:.2f}s)")

    if all_pass:
        print("\nAll tests passed successfully.")
    else:
        print("\nSome tests failed or showed regressions.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--save", help="Save results to file")
    parser.add_argument("--compare", help="Compare against baseline file")
    args = parser.parse_args()

    runner = BenchmarkRunner()
    
    # Define a standard suite with a 5s+ test case (N=5000)
    suite = [
        {"n": 1000, "m": 64, "threads": 1},
        {"n": 1000, "m": 64, "threads": 4},
        {"n": 2000, "m": 64, "threads": 1},
        {"n": 2000, "m": 64, "threads": 4},
        {"n": 5000, "m": 64, "threads": 1},
        {"n": 5000, "m": 64, "threads": 4},
    ]
    
    results = run_suite(runner, suite)
    
    if args.save:
        with open(args.save, "w") as f:
            json.dump(results, f, indent=2)
        print(f"Results saved to {args.save}")
        
    if args.compare:
        if os.path.exists(args.compare):
            with open(args.compare, "r") as f:
                baseline = json.load(f)
            compare_results(baseline, results)
        else:
            print(f"Baseline file {args.compare} not found.")
