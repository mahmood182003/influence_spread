#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <time.h>
#include <functional>

#include <Snap.h>

#define __DEBUG__ 1
#include <utils.h>

using namespace std;
using namespace TSnap;

int SEEDS = 0;
int R;

#define BASE_R 1000

// uncomment at most one, default is GREEDY
//#define PAGERANK 1
//#define HIGHDEGREE 1

class ICModel {
	const MyGraph& G;
	const int R;
	const TStr& dataPath;
	TIntStrH NIdColorH;
	TIntStrH EIdColorH;

	void resetColors() {
		NIdColorH.Clr(true);
		EIdColorH.Clr(true);
		for (EdgeI EI = G->BegEI(); EI != G->EndEI(); EI++) {
			EIdColorH.AddDat(EI.GetId(), "gray"); // default edge color
		}
	}

	// stochastic spread simulation
	size_t diffusion_spread(const vector<NodeI>& seedSet,
			const bool deterministic = false, const bool draw = false) {

		assert(seedSet.size() > 0);
		list<NodeI> q(seedSet.begin(), seedSet.end());
		resetColors();
		map<int, bool> active;
		size_t spread = 0;
		for (NodeI NI : seedSet) {
			active[NI.GetId()] = true;
			NIdColorH.AddDat(NI.GetId(), "yellow");
		}

		int frame = 0;
		if (draw) {
			updateTitle();
			draw_graph(frame++);
		}
		while (!q.empty()) {

			NodeI NI = q.back();
			q.pop_back();
			int out_degree = NI.GetOutDeg();

			TStr color;
			if (draw) {
				// highlight the influencing node
//				color = NIdColorH.GetDat(NI.GetId());
//				NIdColorH.AddDat(NI.GetId(), "green");
			}

//			PRINTF("Node %d, out degree=%d\n", NI.GetId(), out_degree);
			while (out_degree-- > 0) {
				NodeI out_neighbor = G->GetNI(NI.GetOutNId(out_degree));
				if (active.find(out_neighbor.GetId()) != active.end()) { // already active
					continue;
				}

				const int in_degree = out_neighbor.GetInDeg();
//				PRINTF("out neighbor %d has in degree=%d, spread=%d\n",
//						out_neighbor.GetId(), in_degree, spread);
				if (deterministic || rand() % in_degree == 0) { // true with probability 1/in_degree
					++spread;
					q.push_front(out_neighbor);
					active[out_neighbor.GetId()] = true;
					if (draw) {
						NIdColorH.AddDat(NI.GetOutNId(out_degree), "blue");
						EIdColorH.AddDat(NI.GetOutEId(out_degree), "blue");
						updateTitle(spread);
						draw_graph(frame++);
						NIdColorH.AddDat(out_neighbor.GetId(), "black");
						EIdColorH.AddDat(NI.GetOutEId(out_degree), "black");
					}
					assert(spread <= G->GetNodes());
				}
			}
//			NIdColorH.AddDat(NI.GetId(), color); // restore the original color
		}
		return spread;
	}

	// Monte Carlo estimate of expected spread
	float MC_estimate(const vector<NodeI>& seedSet) {
		size_t sum = 0;
		int r = R;
		while (r-- > 0) {
			size_t s = diffusion_spread(seedSet);
			//PRINTF("r=%d, s=%d\n", r, s);
			sum += s;
		}
		return (float) sum / R;
	}
	bool hasNode(const vector<NodeI>& v, const NodeI& NI) {
		for (NodeI _NI : v) {
			if (_NI.GetId() == NI.GetId()) {
				return true;
			}
		}
		return false;
	}

public:
#ifdef PAGERANK
	const char* title_template =
	"spread=%d, ratio=%d%%, nodes:%d, edges:%d, Seed selection: Page-Rank (Berkhin's); Yellow: seed, Black: activated, Blue: activating\n";
#else
#ifdef HIGHDEGREE
	const char* title_template =
	"spread=%d, ratio=%d%%, nodes:%d, edges:%d, Seed selection: High-Degree; Yellow: seed, Black: activated, Blue: activating\n";
#else
	const char* title_template =
			"spread=%d, ratio=%d%%, nodes:%d, edges:%d, Seed selection: Greedy (R=%d); Yellow: seed, Black: activated, Blue: activating\n";
	#endif
#endif
	char title[200];
	size_t spreadLimit = 0;

	int updateTitle(const size_t spread = 0) {
		int ratio = 100 * (float) spread / spreadLimit;
		sprintf(title, title_template, spread, ratio, G->GetNodes(),
				G->GetEdges(), R);
		return ratio;
	}

	ICModel(const MyGraph& G, const int R, const TStr& pathobj) :
			G(G), R(R), dataPath(pathobj) {
		assert(R > 0);
		updateTitle();
	}

	void draw_graph(const int frameNo) {
		TStr imagePath = dataPath + "_/seeds" + to_string(SEEDS).c_str() + "/"
				+ "image.png";
		imagePath = imagePath.GetNumFNm(imagePath, frameNo);

		DrawGViz(G, TGVizLayout::gvlDot, imagePath, title, false, NIdColorH,
				EIdColorH);
	}

	tuple<vector<NodeI>, float> seedSelect_greedy(int k) {
		vector<NodeI> chosen;
		float spread;
		while (k-- > 0) {
			NodeI best = G->BegNI();
			spread = 0;
			for (NodeI NI = G->BegNI(); NI != G->EndNI(); NI++) {
				if (hasNode(chosen, NI)) {
					continue;
				}
				vector<NodeI> testSeeds(chosen.begin(), chosen.end());
				testSeeds.push_back(NI);
				float tmp = MC_estimate(testSeeds);
				if (spread < tmp) {
					spread = tmp;
					best = NI;
				}
			}
			chosen.push_back(best);
		}
		return make_tuple(chosen, spread);
	}

	void seedSelect_pageRank(int k, vector<NodeI>& seeds) {
		PRINTF("using page rank seed selection\n");
		TIntFltH PRankH;
		GetPageRank_v1(G, PRankH);
		PRankH.SortByDat(false);
		for (auto itr = PRankH.BegI(); itr != PRankH.EndI() && k-- > 0; itr++) {
			PRINTF("rank=%f, ", itr.GetDat());
			seeds.push_back(G->GetNI(itr.GetKey()));
		}
	}

	void seedSelect_highDegree(int k, vector<NodeI>& seeds) {
		PRINTF("using highDegree seed selection\n");
		TIntIntH DegreeH;
		for (NodeI NI = G->BegNI(); NI != G->EndNI(); NI++) {
			DegreeH.AddDat(NI.GetId(), NI.GetOutDeg());
		}
		DegreeH.SortByDat(false);
		for (auto itr = DegreeH.BegI(); itr != DegreeH.EndI() && k-- > 0; itr++) {
			seeds.push_back(G->GetNI(itr.GetKey()));
		}
	}

	// the maximum possible spread using the seedSet
	size_t deterministic_spread(const vector<NodeI>& seedSet) {
		return spreadLimit = diffusion_spread(seedSet, true);
	}
	size_t simulate(const vector<NodeI>& seedSet, const bool nodraw = false) {
		return diffusion_spread(seedSet, false, !nodraw);
	}
};

void advance(EdgeI& EI, size_t n) {
	while (n-- > 0) {
		EI++;
	}
}

// scale down the edge set by scale<=1
// always produces same output for an input graph
void sparsify(MyGraph& G, const float scale) {
	assert(scale <= 1);
	srand(1); // deterministic
	size_t b = G->GetEdges() - scale * G->GetEdges();
	// sparsify by deleting edges randomly
	//TRnd(time(NULL)).Randomize(); // this doesn't work!
	vector<int> EI_arr(G->GetEdges()); // for faster access
	size_t i = 0;
	for (EdgeI EI = G->BegEI(); EI != G->EndEI(); EI++) {
		EI_arr[i++] = EI.GetId();
	}
	random_shuffle(EI_arr.begin(), EI_arr.end());
	while (b-- > 0) {
		G->DelEdge(EI_arr[b]);
	}
	srand(time(NULL));
}

int main(int argc, char* argv[]) {
	init();
	if (argv[1]) {
		SEEDS = atoi(argv[1]);
		printf("SEEDS=%d\n", SEEDS);
	} else {
		printf("seed size must be the first argument\n");
		exit(0);
	}
	R = BASE_R / SEEDS; // Monte Carlo iterations

	stringstream result;

	forInput(
			[&result](const string& path) {
				printf(BLUE("\nLoading "));
				printf("%s\n", path.c_str());
				const TStr pathobj(path.c_str());

				MyGraph G = LoadEdgeList<MyGraph>(pathobj, 0, 1);
				PRINTF("%d nodes, %d edges\n",G->GetNodes(),G->GetEdges());

				string name = pathobj.GetFMid().CStr();
				if(SEEDS>G->GetNodes()) {
					PRINTF("skipping this graph\n");
					result << name << '\t'
					<< SEEDS << '\t'
					<< R << '\t'
					<< G->GetNodes()-SEEDS << '\t'
					<< G->GetNodes()-SEEDS << '\t'
					<< 1 << '\t'
					<< 1 << '\t'
					<< G->GetNodes() << '\t'
					<< '\n';
					return;
				}

				const int edgeLimit = 200;
				if(G->GetEdges() > edgeLimit) {
					sparsify(G, (float)edgeLimit / G->GetEdges());
				}
				if(G->GetEdges() > 4*G->GetNodes()) {
					sparsify(G, ((float)4*G->GetNodes()) / G->GetEdges());
				}

				for (NodeI NI = G->BegNI(); NI != G->EndNI(); NI++) {
					if(NI.GetOutDeg()==0 && NI.GetInDeg()==0) {
						G->DelNode(NI.GetId());
					}
				}

				PRINTF("sparsified: %d nodes, %d edges\n",G->GetNodes(),G->GetEdges());

				ICModel model(G, R, pathobj);

#ifdef PAGERANK
			vector<NodeI> seeds;
			model.seedSelect_pageRank(SEEDS, seeds);
			int spread1 = -1, ratio1 = 0;
			model.deterministic_spread(seeds);
#else
#ifdef HIGHDEGREE
			vector<NodeI> seeds;
			model.seedSelect_highDegree(SEEDS, seeds);
			int spread1 = -1, ratio1 = 0;
			model.deterministic_spread(seeds);
#else
			auto [seeds, spread1] = model.seedSelect_greedy(SEEDS);
			model.deterministic_spread(seeds);
			int ratio1 =100*spread1/model.spreadLimit;
			PRINTF("spread=%f/%d, ratio=%d%%, seeds=%d\n", spread1, model.spreadLimit, ratio1, seeds.size() );
#endif
#endif

			PRINTF("drawing and simulation...\n");

			size_t spread2 = model.simulate(seeds,true);
			int ratio2 = model.updateTitle(spread2);
			PRINTF("%s\n", model.title);

			result << name << '\t'
			<< SEEDS << '\t'
			<< R << '\t'
			<< spread1 << '\t'
			<< spread2 << '\t'
			<< ratio1 << '\t'
			<< ratio2 << '\t'
			<< G->GetNodes() << '\t'
			<< '\n';
		});

	std::ofstream outFile("result.txt", std::ios_base::app);
	if (SEEDS == 1) { // due to parallel runs
		outFile << "Name\t"
				<< "Seeds\t"
				<< "R\t"
				<< "\"Expected Spread\"" << '\t'
				<< "\"Simulation Spread\"" << '\t'
				<< "\"Reach Ratio 1\"" << '\t'
				<< "\"Reach Ratio 2\"" << '\t'
				<< "\"#Nodes\"" << '\t'
				<< '\n';
	}
	outFile << result.rdbuf();
	outFile.close();
	return 0;
}
