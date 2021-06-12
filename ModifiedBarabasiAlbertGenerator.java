package Tutorial1;

/*
 * 
 * GraphStream is a library whose purpose is to handle static or dynamic
 * graph, create them from scratch, file or any source and display them.
 *
 */

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;
import java.util.function.Consumer;

import org.graphstream.algorithm.generator.BaseGenerator;

/**
 * Scale-fre graph generator using the preferential attachment rule as defined
 * in the Barabasi-Albert model.
 * 
 * <p>
 * This is a simple graph generator that generates a graph using the
 * preferential attachment rule defined in the Barabasi-Albert model: nodes are
 * generated one by one, and each time attached by one or more edges other
 * nodes. The other nodes are chosen using a biased random selection giving more
 * chance to a node if it has a high degree.
 * </p>
 * 
 * <p>
 * The more this generator is iterated, the more nodes are generated. It can
 * therefore generate graphs of any size. One node is generated at each call to
 * {@link #nextEvents()}. At each node added at least one new edge is added. The
 * number of edges added at each step is given by the
 * {@link #getMaxLinksPerStep()}. However by default the generator creates a
 * number of edges per new node chosen randomly between 1 and
 * {@link #getMaxLinksPerStep()}. To have exactly this number of edges at each
 * new node, use {@link #setExactlyMaxLinksPerStep(boolean)}.
 * </p>
 * 
 * <h2>Complexity</h2>
 * 
 * For each new step, the algorithm act in O(n) with n the number of nodes if 1
 * max edge per new node is created, else the complexity is O(nm) if m max edge
 * per new node is created.
 * 
 */
public class ModifiedBarabasiAlbertGenerator extends BaseGenerator {
	/**
	 * Degree of each node.
	 */
	protected ArrayList<Integer> degrees;

	/**
	 * The maximum number of links created when a new node is added.
	 */
	protected int maxLinksPerStep;

	/**
	 * Does the generator generates exactly {@link #maxLinksPerStep}.
	 */
	protected boolean exactlyMaxLinksPerStep = false;

	/**
	 * The sum of degrees of all nodes
	 */
	protected int sumDeg;

	/**
	 * The sum of degrees of nodes not connected to the new node
	 */
	protected int sumDegRemaining;

	/**
	 * Set of indices of nodes connected to the new node
	 */
	protected Set<Integer> connected;

	/**
	 * New generator.
	 */
	public Random r = new Random();

	public ModifiedBarabasiAlbertGenerator() {
		this(1, false);
	}

	public ModifiedBarabasiAlbertGenerator(int maxLinksPerStep) {
		this(maxLinksPerStep, false);
	}

	public ModifiedBarabasiAlbertGenerator(int maxLinksPerStep, boolean exactlyMaxLinksPerStep) {
		this.directed = true;
		this.maxLinksPerStep = maxLinksPerStep;
		this.exactlyMaxLinksPerStep = exactlyMaxLinksPerStep;
	}

	/**
	 * Maximum number of edges created when a new node is added.
	 * 
	 * @return The maximum number of links per step.
	 */
	public int getMaxLinksPerStep() {
		return maxLinksPerStep;
	}

	/**
	 * True if the generator produce exactly {@link #getMaxLinksPerStep()}, else it
	 * produce a random number of links ranging between 1 and
	 * {@link #getMaxLinksPerStep()}.
	 * 
	 * @return Does the generator generates exactly {@link #getMaxLinksPerStep()}.
	 */
	public boolean produceExactlyMaxLinkPerStep() {
		return exactlyMaxLinksPerStep;
	}

	/**
	 * Set how many edge (maximum) to create for each new node added.
	 * 
	 * @param max The new maximum, it must be strictly greater than zero.
	 */
	public void setMaxLinksPerStep(int max) {
		maxLinksPerStep = max > 0 ? max : 1;
	}

	/**
	 * Set if the generator produce exactly {@link #getMaxLinksPerStep()} (true),
	 * else it produce a random number of links ranging between 1 and
	 * {@link #getMaxLinksPerStep()} (false).
	 * 
	 * @param on Does the generator generates exactly {@link #getMaxLinksPerStep()}.
	 */
	public void setExactlyMaxLinksPerStep(boolean on) {
		exactlyMaxLinksPerStep = on;
	}

	/**
	 * Start the generator. Two nodes connected by edge are added.
	 * 
	 * @see org.graphstream.algorithm.generator.Generator#begin()
	 */
	public void begin() {
		addNode("0");
		addNode("1");
		addEdge("0_1", "0", "1");
		degrees = new ArrayList<Integer>();
		degrees.add(1);
		degrees.add(1);
		sumDeg = 2;
		connected = new HashSet<Integer>();
	}

	/**
	 * Step of the generator. Add a node and try to connect it with some others.
	 * 
	 * The number of links is randomly chosen between 1 and the maximum number of
	 * links per step specified in {@link #setMaxLinksPerStep(int)}.
	 * 
	 * The complexity of this method is O(n) with n the number of nodes if the
	 * number of edges created per new node is 1, else it is O(nm) with m the number
	 * of edges generated per node.
	 * 
	 * @see org.graphstream.algorithm.generator.Generator#nextEvents()
	 */
	public boolean nextEvents() {
		// Generate a new node.
		int nodeCount = degrees.size();
		final String newId = nodeCount + "";
		addNode(newId);

		// Attach to how many existing nodes?
		int n = maxLinksPerStep;
		if (!exactlyMaxLinksPerStep)
			n = random.nextInt(n) + 1;
		n = Math.min(n, nodeCount);

		// Choose the nodes to attach to.
		sumDegRemaining = sumDeg;
		for (int i = 0; i < n; i++)
			chooseAnotherNode();

		connected.forEach(new Consumer<Integer>() {
			public void accept(Integer i) {
				addEdge(i + "_" + newId, i + "", newId);
			//	addEdge(newId + "_" + i, newId, i + "");
				degrees.set(i, degrees.get(i) + 1);
			}
		});

		connected.clear();
		degrees.add(n);
		sumDeg += 2 * n;

		// It is always possible to add an element.
		return true;
	}

	/**
	 * Choose randomly one of the remaining nodes
	 */
	protected void chooseAnotherNode() {
		int r = random.nextInt(sumDegRemaining);
		int runningSum = 0;
		int i = 0;
		while (runningSum <= r) {
			if (!connected.contains(i))
				runningSum += degrees.get(i);
			i++;
		}
		i--;
		connected.add(i);
		sumDegRemaining -= degrees.get(i);
	}

	/**
	 * Clean degrees.
	 * 
	 * @see org.graphstream.algorithm.generator.Generator#end()
	 */
	@Override
	public void end() {
		degrees.clear();
		degrees = null;
		connected = null;
		super.end();
	}
}
