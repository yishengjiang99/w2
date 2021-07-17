// #include <stdio.h>
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
static const char LogTable256[256] =
		{
				-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
				LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
				LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)};
unsigned log2ff(unsigned int v)
{

	unsigned r;									 // r will be lg(v)
	register unsigned int t, tt; // temporaries
	if ((tt = (v >> 16)) > 0)
	{
		r = ((t = tt >> 8)) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	}
	else
	{
		r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
	}
	return r;
}

// int main()
// {
// 	log2ff(3614);
// 	int k = 0;
// 	printf("%u", log2ff(k += 444));
// 	printf("%u", log2ff(k += 444));
// 	printf("%u", log2ff(k += 444));
// 	printf("%u", log2ff(k += 444));
// 	printf("%u", log2ff(k += 444));
// }