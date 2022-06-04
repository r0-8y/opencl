__kernel void num_of_primes(global int *numbers, global int *results, const int n, const int num_of_threads)
{
	int id = get_global_id(0);

	bool is_prime;
	int primes_in_task = 0;
	for(int i = id; i < n; i+=num_of_threads)
	{
		if (i < 2)
		{
			continue;
		}
		is_prime = true;
		for (int j = 2; j <= i/2; j++) {
			if (i % j == 0) {
				is_prime = false;
				break;
			}
		}
		if(is_prime)
		{
			primes_in_task++;
		}
	}

	results[id] = primes_in_task;
}