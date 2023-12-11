import matplotlib.pyplot as plt

# Step 1: Read data from file
filename = 'scores.txt'

# Data structures to store the parsed data
queries = []
ndcg_at_5 = []
ndcg_at_10 = []
ndcg_at_50 = []

with open(filename, 'r') as file:
    lines = file.readlines()
    for i in range(0, len(lines)-4, 4):  # assuming there are 4 lines for each query block
        # Parse query number
        query_line = lines[i].strip()
        query_num = int(query_line.split(': ')[1])
        queries.append(query_num)
        
        # Parse nDCG@5
        ndcg_5_line = lines[i + 1].strip()
        ndcg_5 = float(ndcg_5_line.split(': ')[1])
        ndcg_at_5.append(ndcg_5)
        
        # Parse nDCG@10
        ndcg_10_line = lines[i + 2].strip()
        ndcg_10 = float(ndcg_10_line.split(': ')[1])
        ndcg_at_10.append(ndcg_10)
        
        # Parse nDCG@50
        ndcg_50_line = lines[i + 3].strip()
        ndcg_50 = float(ndcg_50_line.split(': ')[1])
        ndcg_at_50.append(ndcg_50)

# Additional data structures for the overall scores
overall_scores = []

# Assume that the overall scores are the last four lines of the file
for line in lines[-4:]:
    parts = line.split()
    metric = parts[0]
    score = float(parts[-1])
    overall_scores.append((metric, score))

# Step 2: Create the plot
fig, ax = plt.subplots(figsize=(14, 8))

# Assuming you want to create a bar chart for each metric (nDCG@5, nDCG@10, nDCG@50)
barWidth = 0.25
r1 = range(len(queries))
r2 = [x + barWidth for x in r1]
r3 = [x + barWidth for x in r2]

ax.bar(r1, ndcg_at_5, color='b', width=barWidth, edgecolor='grey', label='nDCG@5')
ax.bar(r2, ndcg_at_10, color='r', width=barWidth, edgecolor='grey', label='nDCG@10')
ax.bar(r3, ndcg_at_50, color='g', width=barWidth, edgecolor='grey', label='nDCG@50')

# Add labels, title, and custom x-axis tick labels, etc.
ax.set_xlabel('Query', fontweight='bold', fontsize=15)
ax.set_ylabel('Scores', fontweight='bold', fontsize=15)
ax.set_xticks([r + barWidth for r in range(len(queries))], queries)

columns = ('Metric', 'Score')
cell_text = []
for row in overall_scores:
    cell_text.append([row[0], '{:0.4f}'.format(row[1])])
table = ax.table(cellText=cell_text, colLabels=columns, loc='bottom')

ax.axhline(y=0, color='black') 
# Modifying the layout to accommodate the table
plt.subplots_adjust(bottom=0.2)  # Make space for the table

# Customize the table
table.auto_set_font_size(False)
table.set_fontsize(9)
table.scale(1, 1.5)  # Increase the row height


ax.set_title('nDCG Scores Comparison')
ax.legend()

# Show the plot
plt.tight_layout()
plt.savefig("results.png")
plt.show()

