__kernel void calculate_pi(global double *results, const unsigned int n, const int num_of_threads)
{
	int id = get_global_id(0);
	double h, sum, x;

	h = 1.0 / (double) n;
	sum = 0.0;

	for(int i = id; i <= n; i+=num_of_threads)
	{
		x = h * ((double)i - 0.5);
		sum += 4.0 / (1.0 + x*x);
	}

	results[id] = h * sum;
}