import matplotlib.pyplot as plt
import sys

piece_counts = [0] * 33
phase_counts = [0] * 25
pawn_counts = [0] * 17
pawn_phase_counts = [[0] * 17 for i in range(33)]


if __name__ == "__main__":
    stats_files = sys.argv[1:]
    for stats_filename in stats_files:
        with open(stats_filename, 'r') as stats_file:
            lines = stats_file.readlines()
            curr = 0
            for piece_count in range(0, 33):
                piece_counts[piece_count] += int(lines[curr])
                curr += 1
                
            for phase in range(0, 25):
                phase_counts[phase] += int(lines[curr])
                curr += 1
                
            for pawn_count in range(0, 17):
                pawn_counts[pawn_count] += int(lines[curr])
                curr += 1
                
            for phase in range(0, 25):
                for pawn_count in range(0, 17):
                    pawn_phase_counts[phase][pawn_count] += int(lines[curr])
                    curr += 1
    
    plt.figure(figsize=(10, 4))
    plt.bar(range(0, 33), piece_counts)
    plt.xlabel("Pieces")
    plt.ylabel("Count")
    plt.xticks(range(0, 33, 2))
    plt.show()

    plt.figure(figsize=(10, 4))
    plt.bar(range(0, 25), phase_counts)
    plt.xlabel("Phase")
    plt.ylabel("Count")
    plt.xticks(range(0, 25, 2))
    plt.show()

    plt.figure(figsize=(10, 4))
    plt.bar(range(0, 17), pawn_counts)
    plt.xlabel("Pawns")
    plt.ylabel("Count")
    plt.xticks(range(0, 17, 2))
    plt.show()

    for phase in range(0, 25, 5):
        # ax5 not needed
        fig, axes = plt.subplots(3, 2, figsize=(10,4))
        for (idx, axis) in enumerate(axes.flatten()[0:5]):
            axis.bar(range(0, 17), pawn_phase_counts[phase + idx])
            axis.set_title(f"Pawn counts for phase {phase + idx}")
            axis.set_xlabel("Pawns")
            axis.set_ylabel(f"Count (phase {phase + idx})")
            axis.set_xticks(range(0, 17, 2))
        # plt.bar(range(0, 9), pawn_phase_counts[phase])
        # plt.xlabel("Pawns")
        # plt.ylabel(f"Count (phase {phase})")
        # plt.xticks(range(0, 9))
        plt.subplots_adjust(wspace=0.5, hspace=0.5)
        plt.show()