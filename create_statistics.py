import json
import glob
from pprint import pprint
import matplotlib.pyplot as plt
import argparse


def create_args():
    parser = argparse.ArgumentParser(description='Create experiments graphs.')
    parser.add_argument("--save_files",
            help = "if set graphs are saved to stats/")
    parser.add_argument("reportid",
            help = "will search for reports in <report_dir>/*<reportid>*.json")
    parser.add_argument("report_type",
            help = "must be edge_report | stretch_report | density_report ")
    parser.add_argument("reports_dir", help = "directory of reports")
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
    return [ k * (x ** (1.0 + 1.0 / k)) for x in num_vertices] 

def get_report_id(edge_report):
    return 'edgegraph_{0}_{1}_png'.format(
            edge_report['k'], int(edge_report['density'] * 10))
def get_report_info(edge_report):
    if edge_report['k'] < 0: # three_spanner algorithm
        alg_description = "Three - Spanner"
    else:
        alg_description = "2k - 1 Spanner, k = {0}".format(edge_report['k'])
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

def create_max_stretch_report(stretch_report):
    f = plt.figure()
    f.suptitle(get_report_info(stretch_report[0]), fontsize=14, fontweight='bold')

    num_vertices, num_edges = \
        get_x_y(stretch_report, 'size', 'max_stretch')
    k = stretch_report[0]['k']
    k = k if k > 0 else 2 # if k < 0 this means it was three_spanner.
    report_plot, = plt.plot(num_vertices, num_edges, "r", label = 'spanner alg')
    expected_ys = [3 for x in num_vertices]
    expected_plot, = plt.plot(num_vertices, expected_ys, "g", label = 'k)')
    plt.legend(handles=[report_plot, expected_plot])
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=18.)
    plt.ylabel('max_stretch')
    plt.xlabel('# number_of_vertices')
    if args.save_files:
        den = int (10 * stretch_report[0]['density'])
        start_v = num_vertices[0]
        end_v = num_vertices[-1]
        print den, start_v, end_v
        plt.savefig(
                '/Users/bilalSaad/Desktop/tex/2k_spanner/stretch_report_k{0}_{1}_{2}_{3}.png'.
                format(k, num_vertices[0], num_vertices[-1], den))
        print 'save files'
    else:
        plt.show()
    plt.close()

def create_density_report(edge_report):
    f = plt.figure()
    f.suptitle("Density Graph", fontsize=14, fontweight='bold')

    num_vertices, num_edges = \
        get_x_y(edge_report, 'density', 'average_spanner_size')
    k = edge_report[0]['k']
    expected_ys = get_expected_edge_number(num_vertices, k)
    report_plot, = plt.plot(num_vertices, num_edges, "r", label = 'spanner alg')
    expected_plot, = plt.plot(num_vertices, expected_ys, "g",
            label = 'O(kn^(1+1/k))')
    plt.legend(handles=[report_plot, expected_plot])
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=18.)
    plt.ylabel('# number_of_edges')
    plt.xlabel('density')
    if args.save_files:
        sz = edge_report[0]["size"]
        plt.savefig(
                '/Users/bilalSaad/Desktop/tex/2k_spanner/den_report_k{0}_{1}.png'.
                format(k, sz))
        print 'save files'
    else:
        plt.show()
    plt.close()

def create_stretch_density_report(edge_report):
    f = plt.figure()
    f.suptitle("Density Graph", fontsize=14, fontweight='bold')

    num_vertices, num_edges = \
        get_x_y(edge_report, 'density', 'max_stretch')
    k = edge_report[0]['k']
    report_plot, = plt.plot(num_vertices, num_edges, "r", label = 'spanner alg')
    plt.legend(handles=[report_plot])
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=18.)
    plt.ylabel('max_stretch')
    plt.xlabel('density')
    if args.save_files:
        sz = edge_report[0]["size"]
        plt.savefig(
                '/Users/bilalSaad/Desktop/tex/2k_spanner/stretch_den_report_k{0}_{1}.png'.
                format(k, sz))
        print 'save files'
    else:
        plt.show()
    plt.close()

def edge_reports(pattern = 'build/out/EdgeReport*'):
    edge_reports = glob.glob(pattern)
    for edge_report in edge_reports:
        create_edge_number_graph(get_json_array(edge_report))

def stretch_reports(pattern = 'build/out/EdgeReport*'):
    stretch_reports = glob.glob(pattern)
    for stretch_report in stretch_reports:
        print stretch_report
        create_max_stretch_report(get_json_array(stretch_report))

def density_reports(pattern):
    density_reports = glob.glob(pattern)
    for report in density_reports:
        create_density_report(get_json_array(report))
def stretch_density_reports(pattern):
    density_reports = glob.glob(pattern)
    for report in density_reports:
        create_stretch_density_report(get_json_array(report))
def get_report_function(report_type):
    if report_type == "edge_report":
        return edge_reports
    elif report_type == "stretch_report":
        return stretch_reports
    elif report_type == "density_report":
        return density_reports
    else :
        print 'error unrecognized report type'
if __name__ == "__main__":
    report_func = get_report_function(args.report_type)
    report_func('{0}/*{1}*'.format(args.reports_dir, args.reportid))
