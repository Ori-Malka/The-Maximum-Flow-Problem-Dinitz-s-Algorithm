package Tutorial1;

import org.graphstream.graph.*;
import org.graphstream.graph.implementations.AdjacencyListGraph;

import java.util.Random;

import org.graphstream.algorithm.generator.Generator;

public class MyGraphGenerator {
	public static void main(String args[]) {
		System.setProperty("org.graphstream.ui", "swing");
		Random r = new Random();
		int V = 10000 - 2; // nodes amount
		Graph graph = new AdjacencyListGraph("Graphi");
		// Between 1 and 1000 new links per node added.
		Generator gen = new ModifiedBarabasiAlbertGenerator(800);
		// Generate V nodes:
		gen.addSink(graph);
		gen.begin();
		for (int i = 0; i < V; i++) {
			gen.nextEvents();
		}
		gen.end();
	
		int n = graph.getNodeCount();
		
		// prints the adjacency matrix + adds edges (j,i) in 50% chance where (i,j)
		// doesn't exist.
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				byte temp = (byte) ((graph.getNode(i + "").hasEdgeToward(j + "")) ? 1 : 0);
				System.out.print(temp);
				if ((temp == 0) && (i < j) && (j != n - 1) && (i != 0) && r.nextInt(2) == 0)
					graph.addEdge(j + "_" + i, j + "", i + "", true);

			}
			System.out.println();
		}
		System.out.println(n+" "+graph.getEdgeCount());
	}

}