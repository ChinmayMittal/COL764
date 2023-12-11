import matplotlib.pyplot as plt
import seaborn as sns


### dirichilet smoothing
# x = [50, 100, 200, 250, 300, 400]
# average =     [0.1402, 0.1404, 0.1409, 0.1412, 0.1409, 0.1406]
# cut_5_data =  [0.6418, 0.6298, 0.6047, 0.6152, 0.6095, 0.6050]
# cut_10_data = [0.6026, 0.6029, 0.6030, 0.6050, 0.6071, 0.5952]
# cut_50_data = [0.5120, 0.5163, 0.5211, 0.5222, 0.5210, 0.5162]

### expansion
# x =           [5, 7, 10, 15, 20]
# average =     [0.1402, 0.1403, 0.1405, 0.1397, 0.1400]
# cut_5_data =  [0.6377, 0.6475, 0.6396, 0.6256, 0.6375]
# cut_10_data = [0.5924, 0.5949, 0.5970, 0.5966, 0.5985]
# cut_50_data = [0.4971, 0.4971, 0.4987, 0.4973, 0.4984]

### interpolation
x =           [0.1, 0.25, 0.3, 0.4, 0.5, 0.75, 1]
average =     [0.1389, 0.1400, 0.1400, 0.1400, 0.1404, 0.1400, 0.1390]
cut_5_data =  [0.6182, 0.6375, 0.6367, 0.6360, 0.6466, 0.6283, 0.5998]
cut_10_data = [0.5838, 0.5985, 0.5970, 0.5844, 0.5879, 0.5743, 0.5746]
cut_50_data = [0.4920, 0.4984, 0.4980, 0.4971, 0.4971, 0.4876, 0.4804]

plt.figure(figsize=(10, 6))


sns.set(style="whitegrid")


plt.plot(x, average, label='NDCG', linestyle='--', marker='x', color='black', markersize=4)
plt.plot(x, cut_5_data, label='NDCG@5', linestyle='-.', marker='x', color='blue', markersize=4)
plt.plot(x, cut_10_data, label='NDCG@10', linestyle='-', marker='x', color='g', markersize=4)
plt.plot(x, cut_50_data, label='NDCG@50', linestyle=':', marker='x', color='r', markersize=4)

# Customize axes and labels
plt.xlabel('Interpolation Constant', fontsize=12, fontweight='bold')
plt.ylabel('NDCG', fontsize=12, fontweight='bold')
plt.title('Effect of Query Expansion Interpolation Constant', fontsize=16, fontweight='bold')

# Set the limit for the axes if required
# plt.xlim([xmin, xmax])
# plt.ylim([ymin, ymax])


plt.legend(title='Legends', title_fontsize='13', fontsize='12')

# Remove the top and right spines from plot
sns.despine()

# Adjust layout
plt.tight_layout()

# Show plot
plt.savefig("interpolation.png")
plt.show()