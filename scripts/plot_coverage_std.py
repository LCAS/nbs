import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as plticker


# 1) Load all the data for the 5 environments
# Orebro
mcdm_data_1            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/orebro/coverage_mcdm_orebro_33_1.csv", "rb"), delimiter=",", skiprows=1)
mcdm_data_2            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/orebro/coverage_mcdm_orebro_33_2.csv", "rb"), delimiter=",", skiprows=1)
mcdm_data_3            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/orebro/coverage_mcdm_orebro_33_3.csv", "rb"), delimiter=",", skiprows=1)
random_frontier_data_1 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/orebro/coverage_random_frontier_orebro_9.csv", "rb"), delimiter=",", skiprows=1)
random_frontier_data_2 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/orebro/coverage_random_frontier_orebro_52.csv", "rb"), delimiter=",", skiprows=1)
random_frontier_data_3 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/orebro/coverage_random_frontier_orebro_83.csv", "rb"), delimiter=",", skiprows=1)
random_walk_data_1     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/orebro/coverage_random_walk_orebro_9.csv", "rb"), delimiter=",", skiprows=1)
random_walk_data_2     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/orebro/coverage_random_walk_orebro_52.csv", "rb"), delimiter=",", skiprows=1)
random_walk_data_3     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/orebro/coverage_random_walk_orebro_83.csv", "rb"), delimiter=",", skiprows=1)
# NCFM
# mcdm_data_1            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/ncfm/coverage_mcdm_ncfm_30_1.csv", "rb"), delimiter=",", skiprows=1)
# mcdm_data_2            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/ncfm/coverage_mcdm_ncfm_30_2.csv", "rb"), delimiter=",", skiprows=1)
# mcdm_data_3            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/ncfm/coverage_mcdm_ncfm_30_3.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_1 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/ncfm/coverage_random_frontier_iliad_9.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_2 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/ncfm/coverage_random_frontier_iliad_52.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_3 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/ncfm/coverage_random_frontier_iliad_83.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_1     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/ncfm/coverage_random_walk_ncfm_9.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_2     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/ncfm/coverage_random_walk_ncfm_52.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_3     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/ncfm/coverage_random_walk_ncfm_83.csv", "rb"), delimiter=",", skiprows=1)
# INB3123
# mcdm_data_1            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_atrium/coverage_mcdm_inb_atrium_33_1.csv", "rb"), delimiter=",", skiprows=1)
# mcdm_data_2            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_atrium/coverage_mcdm_inb_atrium_33_2.csv", "rb"), delimiter=",", skiprows=1)
# mcdm_data_3            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_atrium/coverage_mcdm_inb_atrium_33_3.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_1 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_atrium/coverage_random_frontier_inb_atrium_9.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_2 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_atrium/coverage_random_frontier_inb_atrium_52.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_3 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_atrium/coverage_random_frontier_inb_atrium_83.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_1     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_atrium/coverage_random_walk_inb_atrium_9.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_2     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_atrium/coverage_random_walk_inb_atrium_52.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_3     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_atrium/coverage_random_walk_inb_atrium_83.csv", "rb"), delimiter=",", skiprows=1)
# # INB_ENG
# mcdm_data_1            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_eng/coverage_mcdm_inb_eng_33_1.csv", "rb"), delimiter=",", skiprows=1)
# mcdm_data_2            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_eng/coverage_mcdm_inb_eng_33_2.csv", "rb"), delimiter=",", skiprows=1)
# mcdm_data_3            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_eng/coverage_mcdm_inb_eng_33_3.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_1 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_eng/coverage_random_frontier_inb_eng_9.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_2 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_eng/coverage_random_frontier_inb_eng_52.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_3 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_eng/coverage_random_frontier_inb_eng_83.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_1     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_eng/coverage_random_walk_inb_eng_9.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_2     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_eng/coverage_random_walk_inb_eng_52.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_3     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_eng/coverage_random_walk_inb_eng_83.csv", "rb"), delimiter=",", skiprows=1)
# # INB_Atrium
# mcdm_data_1            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_atrium/coverage_mcdm_inb_atrium_33_1.csv", "rb"), delimiter=",", skiprows=1)
# mcdm_data_2            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_atrium/coverage_mcdm_inb_atrium_33_2.csv", "rb"), delimiter=",", skiprows=1)
# mcdm_data_3            = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/mcdm/logic_exps/inb_atrium/coverage_mcdm_inb_atrium_33_3.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_1 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_atrium/coverage_random_frontier_inb_atrium_9.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_2 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_atrium/coverage_random_frontier_inb_atrium_52.csv", "rb"), delimiter=",", skiprows=1)
# random_frontier_data_3 = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_frontier/inb_atrium/coverage_random_frontier_inb_atrium_83.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_1     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_atrium/coverage_random_walk_inb_atrium_9.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_2     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_atrium/coverage_random_walk_inb_atrium_52.csv", "rb"), delimiter=",", skiprows=1)
# random_walk_data_3     = np.loadtxt(open("/home/pulver/Dropbox/University/PostDoc/MCDM/random_walk/inb_atrium/coverage_random_walk_inb_atrium_83.csv", "rb"), delimiter=",", skiprows=1)

# Remove the first 4 columns containing the weights value
mcdm_data_1 = mcdm_data_1[:, -2:]
mcdm_data_2 = mcdm_data_2[:, -2:]
mcdm_data_3 = mcdm_data_3[:, -2:]
random_frontier_data_1 = random_frontier_data_1[:, -2:]
random_frontier_data_2 = random_frontier_data_2[:, -2:]
random_frontier_data_3 = random_frontier_data_3[:, -2:]
random_walk_data_1 = random_walk_data_1[:, -2:]
random_walk_data_2 = random_walk_data_2[:, -2:]
random_walk_data_3 = random_walk_data_3[:, -2:]

# Remove duplicare rows
unique_1 = np.unique(mcdm_data_1, axis=0)
unique_2 = np.unique(mcdm_data_2, axis=0)
unique_3 = np.unique(mcdm_data_3, axis=0)
unique_rf_1 = np.unique(random_frontier_data_1, axis=0)
unique_rf_2 = np.unique(random_frontier_data_2, axis=0)
unique_rf_3 = np.unique(random_frontier_data_3, axis=0)
unique_rw_1 = np.unique(random_walk_data_1, axis=0)
unique_rw_2 = np.unique(random_walk_data_2, axis=0)
unique_rw_3 = np.unique(random_walk_data_3, axis=0)

# Make the trajectories long the same length and store it in a single array
len_arr = [unique_1.shape[0], unique_2.shape[0], unique_3.shape[0]]
max_len_index = np.argmax(len_arr)
final = np.zeros(shape=(np.max(len_arr), 5))
arr_list = [unique_1, unique_2, unique_3]
for i in range(3):
    while arr_list[i].shape[0] < np.max(len_arr):
        arr_list[i] = np.vstack([arr_list[i], arr_list[i][-1,:]])
    final[:,i] = arr_list[i][:,1]

len_rf_arr = [unique_rf_1.shape[0], unique_rf_2.shape[0], unique_rf_3.shape[0]]
max_len_index = np.argmax(len_rf_arr)
final_rf = np.zeros(shape=(np.max(len_rf_arr), 5))
arr_rf_list = [unique_rf_1, unique_rf_2, unique_rf_3]
for i in range(3):
    while arr_rf_list[i].shape[0] < np.max(len_rf_arr):
        arr_rf_list[i] = np.vstack([arr_rf_list[i], arr_rf_list[i][-1,:]])
    final_rf[:,i] = arr_rf_list[i][:,1]

len_rw_arr = [unique_rw_1.shape[0], unique_rw_2.shape[0], unique_rw_3.shape[0]]
max_len_index = np.argmax(len_rw_arr)
final_rw = np.zeros(shape=(np.max(len_rw_arr), 5))
arr_rw_list = [unique_rw_1, unique_rw_2, unique_rw_3]
for i in range(3):
    while arr_rw_list[i].shape[0] < np.max(len_rw_arr):
        arr_rw_list[i] = np.vstack([arr_rw_list[i], arr_rw_list[i][-1,:]])
    final_rw[:,i] = arr_rw_list[i][:,1]

# Calculate average and std dev from the trajectories
final[:,3] = np.average(final[:,0:3], axis=1)
final[:,4] = np.std(final[:,0:3], axis=1)
final_rf[:,3] = np.average(final_rf[:,0:3], axis=1)
final_rf[:,4] = np.std(final_rf[:,0:3], axis=1)
final_rw[:,3] = np.average(final_rw[:,0:3], axis=1)
final_rw[:,4] = np.std(final_rw[:,0:3], axis=1)
# print(final[:,4])

# Plot the trajectories
x = np.arange(1, final.shape[0] + 1, 1)
print("x: ", len(x))
print(final.shape[0])
plt.plot(x, final[:, 3],  color='#CC4F1B', label='NBS')
plt.fill_between(x, final[:, 3]-final[:, 4], final[:, 3]+final[:, 4], alpha=0.5, edgecolor='#CC4F1B', facecolor='#FF9848')

x = np.arange(1, final_rf.shape[0] + 1, 1)
print("x: ", len(x))
print(final_rf.shape[0])
plt.plot(x, final_rf[:, 3],  color='#1B2ACC', label='RandomFrontier')
plt.fill_between(x, final_rf[:, 3]-final_rf[:, 4], final_rf[:, 3]+final_rf[:, 4], alpha=0.5, edgecolor='#1B2ACC', facecolor='#089FFF')

x = np.arange(1, final_rw.shape[0] + 1, 1)
print("x: ", len(x))
print(final_rw.shape[0])
plt.plot(x, final_rw[:, 3],  color='#3F7F4C', label='RandomWalk')
plt.fill_between(x, final_rw[:, 3]-final_rw[:, 4], final_rw[:, 3]+final_rw[:, 4], alpha=0.5, edgecolor='#3F7F4C', facecolor='#7EFF99')

# Draw vertical lines at the enf of the plot
plt.axvline(x=final.shape[0] + 1, color='#CC4F1B', linestyle="--")
plt.axvline(x=final_rf.shape[0] + 1, color='#1B2ACC', linestyle="--")


plt.legend( ncol=3, bbox_to_anchor=(1, 1.10))
axes = plt.gca()
axes.set_xlim([0, 1000])
axes.set_ylim([0, 100])
axes.set_xlabel("Robot Configuration")
axes.set_ylabel("Map Coverage")
plt.show()