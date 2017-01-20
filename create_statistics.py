
import json
from pprint import pprint
import matplotlib.pyplot as plt


def get_x_y(json_array, xx, yy):
    num_vars = [x[xx] for x in json_array]
    results = [x[yy] for x in json_array]
    return num_vars, results

def get_json_array(fname):
    data = []
    with open(fname) as data_file:
        data = json.load(data_file)
    return data

def edge_number(file_name = 'build/3_spanner_big_graphsgraph_report.json'):
    data_graph_runs = get_json_array(file_name);
    local_x, local_y = get_x_y(data_graph_runs, 'size', 'average_spanner_size')
    expected_ys = [3 * x ** 1.5 for x in local_x]
    local_plot, = plt.plot(local_x, local_y, "r", label = '3 spanner alg')
    expected_plot, = plt.plot(local_x, expected_ys, "g", label = '3x^1.5')
    plt.legend(handles=[local_plot, expected_plot])
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=18.)
    plt.ylabel('# number_of_edges')
    plt.xlabel('# number_of_vertices')
    plt.show()
    return plt
def stretch_factor(file_name = 'build/3_spanner_strechStretchReport.json'):
    data_graph_runs = get_json_array(file_name);
    local_x, local_y = get_x_y(data_graph_runs, 'size', 'average_stretch')
    local_plot, = plt.plot(local_x, local_y, "r", label = '3 spanner alg')
    plt.legend(handles=[local_plot])
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=18.)
    plt.ylabel('# stretch factor')
    plt.xlabel('# number_of_vertices')
    plt.show()

def compare_2k_3(kfname = 'build/2k_Agraph_report.json',
        fname = 'build/3_spanner_Bgraph_report.json'):
    two_k_data, three_data = get_json_array(kfname) , get_json_array(fname)
    local_x, local_y = get_x_y(three_data, 'size', 'average_spanner_size')
    k_x, k_y = get_x_y(two_k_data, 'size', 'average_spanner_size')
    expected_ys = [3 * x ** 1.5 for x in local_x]
    local_plot, = plt.plot(local_x, local_y, "r", label = '3 spanner alg')
    k_plot, = plt.plot(k_x, k_y, "b", label = '2k_spanner')
    expected_plot, = plt.plot(local_x, expected_ys, "g", label = '3x^1.5')
    plt.legend(handles=[local_plot, expected_plot, k_plot])
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=18.)
    plt.ylabel('# number_of_edges')
    plt.xlabel('# number_of_vertices')
    plt.show()

compare_2k_3()
