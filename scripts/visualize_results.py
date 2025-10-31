#!/usr/bin/env python3
"""
Simple visualization script for Network Simulator results
"""

import matplotlib
matplotlib.use('Agg')  # Non-interactive backend
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os

# Ensure results directory exists
os.makedirs('results', exist_ok=True)

def visualize_scenario(csv_file):
    """Visualize a single scenario's results"""
    print(f"\nProcessing: {csv_file}")
    
    # Read CSV
    df = pd.read_csv(csv_file)
    scenario_name = os.path.basename(csv_file).replace('.csv', '')
    
    print(f"  Data shape: {df.shape}")
    print(f"  Columns: {list(df.columns)[:5]}...")
    
    # Create figure with subplots
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle(f'Network QoS Simulation - {scenario_name}', fontsize=16, fontweight='bold')
    
    # 1. Throughput over time
    ax = axes[0, 0]
    flow_throughput_cols = [col for col in df.columns if 'Flow' in col and 'Throughput' in col]
    for col in flow_throughput_cols:
        flow_num = col.split('_')[0].replace('Flow', '')
        ax.plot(df['Timestamp'], df[col], label=f'Flow {flow_num}', linewidth=2)
    ax.set_xlabel('Time (seconds)', fontsize=11)
    ax.set_ylabel('Throughput (Bytes/sec)', fontsize=11)
    ax.set_title('Per-Flow Throughput Over Time', fontsize=12, fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # 2. Queue Occupancy
    ax = axes[0, 1]
    ax.plot(df['Timestamp'], df['QueueOccupancy'], color='#e74c3c', linewidth=2)
    ax.fill_between(df['Timestamp'], df['QueueOccupancy'], alpha=0.3, color='#e74c3c')
    ax.set_xlabel('Time (seconds)', fontsize=11)
    ax.set_ylabel('Queue Size (packets)', fontsize=11)
    ax.set_title('Queue Occupancy Over Time', fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    
    # 3. Drop Rates
    ax = axes[1, 0]
    drop_rate_cols = [col for col in df.columns if 'DropRate' in col]
    for col in drop_rate_cols:
        flow_num = col.split('_')[0].replace('Flow', '')
        ax.plot(df['Timestamp'], df[col], label=f'Flow {flow_num}', linewidth=2)
    ax.set_xlabel('Time (seconds)', fontsize=11)
    ax.set_ylabel('Drop Rate (%)', fontsize=11)
    ax.set_title('Packet Drop Rate Over Time', fontsize=12, fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # 4. Average Delays
    ax = axes[1, 1]
    delay_cols = [col for col in df.columns if 'Delay' in col]
    for col in delay_cols:
        flow_num = col.split('_')[0].replace('Flow', '')
        ax.plot(df['Timestamp'], df[col], label=f'Flow {flow_num}', linewidth=2)
    ax.set_xlabel('Time (seconds)', fontsize=11)
    ax.set_ylabel('Delay (ms)', fontsize=11)
    ax.set_title('Average Packet Delay Over Time', fontsize=12, fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # Save
    output_file = f'results/{scenario_name}_visualization.png'
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ Saved: {output_file}")
    plt.close()

def create_summary_comparison():
    """Create comparison chart across all scenarios"""
    print("\nCreating summary comparison...")
    
    scenarios = ['scenario1_stats.csv', 'scenario2_stats.csv', 'scenario3_stats.csv']
    scenario_names = ['Scenario 1\n(Basic)', 'Scenario 2\n(Priority QoS)', 'Scenario 3\n(Bursty Traffic)']
    
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    fig.suptitle('Network QoS Simulation - Scenario Comparison', fontsize=16, fontweight='bold')
    
    for idx, (csv_file, name) in enumerate(zip(scenarios, scenario_names)):
        if not os.path.exists(f'results/{csv_file}'):
            continue
            
        df = pd.read_csv(f'results/{csv_file}')
        
        # Extract final statistics
        last_row = df.iloc[-1]
        
        # Get throughput for each flow
        throughput_cols = [col for col in df.columns if 'Throughput' in col and 'Flow' in col]
        throughputs = [last_row[col] / 1024 for col in throughput_cols]  # Convert to KB/s
        flow_labels = [f'Flow {i+1}' for i in range(len(throughputs))]
        
        # Plot
        ax = axes[idx]
        colors = ['#3498db', '#e74c3c', '#2ecc71']
        bars = ax.bar(flow_labels, throughputs, color=colors[:len(throughputs)], 
                     edgecolor='black', linewidth=1.5)
        
        # Add value labels
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height,
                   f'{height:.1f}',
                   ha='center', va='bottom', fontsize=10, fontweight='bold')
        
        ax.set_ylabel('Throughput (KB/s)', fontsize=11)
        ax.set_title(name, fontsize=12, fontweight='bold')
        ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    output_file = 'results/scenario_comparison.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ Saved: {output_file}")
    plt.close()

def main():
    print("=" * 70)
    print("Network Traffic Shaping & QoS Simulator - Visualization")
    print("=" * 70)
    
    # Visualize each scenario
    for scenario in ['scenario1_stats.csv', 'scenario2_stats.csv', 'scenario3_stats.csv']:
        csv_path = f'results/{scenario}'
        if os.path.exists(csv_path):
            visualize_scenario(csv_path)
        else:
            print(f"Warning: {csv_path} not found")
    
    # Create comparison
    create_summary_comparison()
    
    print("\n" + "=" * 70)
    print("✓ Visualization Complete!")
    print("=" * 70)
    print("\nGenerated files:")
    print("  • results/scenario1_stats_visualization.png")
    print("  • results/scenario2_stats_visualization.png")
    print("  • results/scenario3_stats_visualization.png")
    print("  • results/scenario_comparison.png")
    print("\nAll graphs saved to results/ directory")
    print("=" * 70)

if __name__ == "__main__":
    main()
