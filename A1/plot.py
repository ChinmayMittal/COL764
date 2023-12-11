import matplotlib.pyplot as plt

# Step 1: Read the numbers from the text file
numbers = []
with open('data.txt', 'r') as file:
    for line in file:
        try:
            numbers.append(float(line.strip()))  # Convert string to int and append to list
        except ValueError:
            # skip invalid lines
            pass

# Step 2: Use matplotlib to plot the histogram
plt.figure(figsize=(10, 6))
plt.hist(numbers, bins=30, color='blue', edgecolor='black', alpha=0.7)

plt.title('Histogram of Relevant Results per Query')
plt.xlabel('Number of Relevant Results')
# plt.xlabel("Average F1@(100,50,20,19)")
plt.ylabel('Number of Queries')

# Enhance grid and layout
plt.grid(axis='y', linestyle='--', alpha=0.7, linewidth=0.5)
plt.tight_layout()

# Show the plot
plt.show()
