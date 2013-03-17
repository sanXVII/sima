#ifndef RTREE_H_
#define RTREE_H_


typedef struct rtree_n
{
} rtree_n;


#define RTREE_NBLOCK_SZ (1000)
typedef struct rtree_nblk
{
  rtree_n node[ RTREE_NBLOCK_SZ ];
  struct rtree_nblk * next;
} rtree_nblk;


typedef struct rtree
{
  rtree_nblk first_blk;
} rtree;


rtree * new_rtree( void );
void del_rtree( rtree * rt );


#endif /* RTREE_H_ */
