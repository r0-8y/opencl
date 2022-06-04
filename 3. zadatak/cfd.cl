__kernel void jacobistep(global double *psinew, global double *psi, const int m, const int n)
{
	int i = get_global_id(0) + 1;
  
	for(int j=1;j<=n;j++) {
		psinew[i*(m+2)+j]=0.25*(psi[(i-1)*(m+2)+j]+psi[(i+1)*(m+2)+j]+psi[i*(m+2)+j-1]+psi[i*(m+2)+j+1]);
	}
}