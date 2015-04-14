#include <map.h>
using namespace std;

namespace import_map{
  
Map::Map(std::ifstream& infile, int resolution)
{
  Map::createMap(infile);
  Map::createGrid(resolution);
}

void Map::createMap(std::ifstream& infile)
{
  int row = 0, col = 0;
  std::stringstream ss;
  std::string inputLine = "";

  // First line : version
  getline(infile,inputLine);
  //if(inputLine.compare("P2") != 0) cerr << "Version error" << endl;
  //else cout << "Version : " << inputLine << endl;

  // Second line : comment
 // getline(infile,inputLine);
 // cout << "Comment : " << inputLine << endl;

  // Continue with a stringstream
  ss << infile.rdbuf();
  // Third line : size
  ss >> numCols >> numRows;
 // cout << numCols << " columns and " << numRows << " rows" << endl;
  
 // Second line : comment
 getline(infile,inputLine);
 //cout << "Comment : " << inputLine << endl;
 
 //std::vector<int> array(numRows*numCols);
 Map::map.reserve(numRows*numCols);

  // Following lines : data
  for(row = 0; row < numRows; ++row)
    for (col = 0; col < numCols; ++col) ss >> Map::map[row*numCols + col];
 
  infile.close();
}

void Map::createGrid(int resolution)
{
  //cluster cells into grid
  int clustersize = (int)((double)(100/resolution));
  Map::numGridRows = numRows/clustersize;
  Map::numGridCols = numCols/clustersize;
  int gridrow = 0, gridcol = 0;
  
  //get the size of the array and initialize to 0
  
  Map::grid.reserve(numGridRows*numGridCols);
  
  for (gridrow = 0; gridrow < numGridRows; ++gridrow)
  {
    for (gridcol = 0; gridcol < numGridCols; ++gridcol)
    {
      grid[gridrow*numGridCols + gridcol] = 0;
    }
  }
  
  
  //set 1 in the grid cells corrisponding to obstacles
  
  for(int row = 0; row < numRows; ++row)
  {
    for(int col = 0; col < numCols; ++col)
    {
      if(map[row*numCols + col] == 0) grid[(int)((float)row/clustersize)*numGridCols + (int)((float)col/clustersize)] = 1;
    }
  }
}



//scans the are around the coordinates passed, starting from the left and going clockwise, then stores the edge points in the edgePoints vector

/*void Map::findFreeEdges(int cX, int cY)
{
  int i, j;
  int range = 10;	//range to scan looking for positions
  int tempX, tempY;
 
  //left edge
  
  for(i = cX; i > cX - range; --i)
  {
    if(getGridValue(i - 1, cY) != 2)
    {
      tempX = i;
      break;
    }
  }
  for(j = cY; j < cY + (int)(range/2); ++j)
  {
    if(getGridValue(tempX, j + 1) != 2)
    {
      tempY = j;
      break;
    }
  }
  
  addEdgePoint(tempX, tempY);
  
  for(j = cY; j > cY - (int)(range/2); --j)
  {
    if(getGridValue(tempX, j - 1) != 2)
    {
      tempY = j;
      break;
    }
  }
  
  addEdgePoint(tempX, tempY);
    
  
  //upper edge
  
  for(j = cY; j > cY - range; --j)
  {
    if(getGridValue(j - 1, cX) != 2)
    {
      tempY = j;
      break;
    }
  }
  for(i = cX; i > cX - (int)(range/2); --i)
  {
    if(getGridValue(i - 1, tempY) != 2)
    {
      tempX = i;
      break;
    }
  }
  
  addEdgePoint(tempX, tempY);
  
  for(i = cX; i < cX + (int)(range/2); ++i)
  {
    if(getGridValue(i + 1, tempY) != 2)
    {
      tempX = i;
      break;
    }
  }
  
  addEdgePoint(tempX, tempY);
  
  
  //right edge
  
  for(i = cX; i < cX + range; ++i)
  {
    if(getGridValue(i + 1, cY) != 2)
    {
      tempX = i;
      break;
    }
  }
  for(j = cY; j > cY - (int)(range/2); --j)
  {
    if(getGridValue(tempX, j - 1) != 2)
    {
      tempY = j;
      break;
    }
  }
  
  addEdgePoint(tempX, tempY);
  
  for(j = cY; j < cY + (int)(range/2); ++j)
  {
    if(getGridValue(tempX, j + 1) != 2)
    {
      tempY = j;
      break;
    }
  }
  
  addEdgePoint(tempX, tempY);
  
  
  //bottom edge
  
  for(j = cY; j < cY + range; ++j)
  {
    if(getGridValue(j + 1, cX) != 2)
    {
      tempY = j;
      break;
    }
  }
  for(i = cX; i < cX + (int)(range/2); ++i)
  {
    if(getGridValue(i + 1, tempY) != 2)
    {
      tempX = i;
      break;
    }
  }
  
  addEdgePoint(tempX, tempY);
  
  for(i = cX; i > cX - (int)(range/2); --i)
  {
    if(getGridValue(i - 1, tempY) != 2)
    {
      tempX = i;
      break;
    }
  }
  
  addEdgePoint(tempX, tempY);
  
}

*/





// values: 0 -> unscanned free cell, 1 -> obstacle cell, 2 -> scanned free cell
void Map::setGridValue(int value, int i, int j)
{
  if(value == 0 || value == 1 || value == 2)
  {
  grid[i*numGridCols + j] = value;
  }
}


void Map::addEdgePoint(int x, int y)
{
  edgePoints.push_back(std::make_pair<int, int>(x, y));
}


//various getters

int Map::getGridValue(int i, int j)
{
  return grid[i*numGridCols + j];
}

int Map::getMapValue(int i, int j)
{
  return map[i*numCols + j];
}


int Map::getNumGridCols()
{
  return numGridCols;
}

int Map::getNumGridRows()
{
  return numGridRows;
}

int Map::getNumCols()
{
  return numCols;
}

int Map::getNumRows()
{
  return numRows;
}



}