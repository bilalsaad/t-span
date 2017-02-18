import json
import glob
from pprint import pprint
import matplotlib.pyplot as plt
import argparse


def create_args():
    parser = argparse.ArgumentParser(description='Create experiments graphs.')
    parser.add_argument("--save_files",
            help = "if set graphs are saved to stats/")
    args = parser.parse_args()
    return args
args = create_args()

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
def stretch_factor(file_name = 'build/5k_AStretchReport.json'):
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
    expected_ys = [2.63 * x ** 1.5 for x in local_x]
    local_plot, = plt.plot(local_x, local_y, "r", label = '3 spanner alg')
    k_plot, = plt.plot(k_x, k_y, "b", label = '2k_spanner')
    expected_plot, = plt.plot(local_x, expected_ys, "g", label = '3x^1.5')
    plt.legend(handles=[local_plot, expected_plot, k_plot])
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=18.)
    plt.ylabel('# number_of_edges')
    plt.xlabel('# number_of_vertices')
    plt.show()

def get_expected_edge_number(num_vertices, k):
    return [ 2 * k * (x ** (1.0 + 1.0 / k)) for x in num_vertices] 

def get_report_id(edge_report):
    return 'edgegraph_{0}_{1}.png'.format(
            edge_report['k'], edge_report['density'])
def get_report_info(edge_report):
    if edge_report['k'] < 0: # three_spanner algorithm
        alg_description = "Three - Spanner"
    else:
        alg_description = "2 * {0} - 1 Spanner".format(edge_report['k'])
    return '{0}, density = {1}'.format(alg_description, edge_report['density'])

def create_edge_number_graph(edge_report):
    f = plt.figure()
    f.suptitle(get_report_info(edge_report[0]), fontsize=14, fontweight='bold')

    num_vertices, num_edges = \
        get_x_y(edge_report, 'size', 'average_spanner_size')
    k = edge_report[0]['k']
    k = k if k > 0 else 2 # if k < 0 this means it was three_spanner.
    expected_ys = get_expected_edge_number(num_vertices, k)
    report_plot, = plt.plot(num_vertices, num_edges, "r", label = 'spanner alg')
    expected_plot, = plt.plot(num_vertices, expected_ys, "g",
            label = 'O(kn^(1+1/k))')
    plt.legend(handles=[report_plot, expected_plot])
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=18.)
    plt.ylabel('# number_of_edges')
    plt.xlabel('# number_of_vertices')
    if args.save_files:
        plt.savefig('stats/' + get_report_id(edge_report[0]))
        print 'save files'
    plt.show()
    plt.close()

def edge_reports(pattern = 'build/out/EdgeReport*'):
    edge_reports = glob.glob(pattern)
    for edge_report in edge_reports:
        create_edge_number_graph(get_json_array(edge_report))



if __name__ == "__main__":
    edge_reports('build/out/*hB2ivDL.json');
