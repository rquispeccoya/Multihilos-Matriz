#include <vector>
#include <iostream>
#include <numeric>
#include <chrono>
#include <thread>
#include <future>
#include <assert.h>

namespace myThread
{
template <typename DataType>
class Matrix //outer class
{
    using RowType = std::vector<DataType>;
    using MatrixType  = std::vector<RowType>;
    MatrixType Data{};
    std::size_t Rows{};
    std::size_t Columns{};

public:
    Matrix();
    Matrix(const std::size_t& rows, const std::size_t& columns);

    const RowType& operator[](unsigned rowIndex) const {return Data[rowIndex];}
    RowType& operator[](unsigned rowIndex) {return Data[rowIndex];}

    //sobrecarga del operador *
    friend Matrix<DataType> operator*(const Matrix<DataType>& matrixA, const Matrix<DataType>& matrixB)
    {
        assert(matrixA.Columns==matrixB.Rows);
        Matrix<DataType> result{matrixA.Rows,matrixB.Columns};

        auto dotProduct=[](const Matrix<DataType>& a,const Matrix<DataType>&b,unsigned row, unsigned col,DataType& sumaTotal){
            for(unsigned i=0;i<a.Columns;++i){
                sumaTotal+=a[row][i]*b[i][col];
            }
        };

        std::vector<std::thread> hilos;
        for(unsigned row=0;row<result.Rows;++row){
            for(unsigned col=0;col<result.Columns;++col){
                hilos.emplace_back(std::thread(dotProduct,std::ref(matrixA),std::ref(matrixB),row,col,std::ref(result[row][col])));
            }
        }

        for(auto&thread:hilos)
        {
            thread.join();
        }
        return result;

    }

    //imprimir matriz
    friend std::ostream & operator <<(std::ostream &output,const Matrix<DataType> &matrix){
        for(unsigned i=0;i<matrix.Rows;++i){
            for(unsigned j=0;j<matrix.Columns-1;++j)
                output <<matrix[i][j]<<"  " ;
            output <<matrix[i][matrix.Columns-1] ;
            if(i==matrix.Rows-1){
            }else{
                output <<std::endl ;
            }
        }
        return output;
    }

    DataType ComputeInnerSum();
    DataType ComputeInnerSumThread();
    DataType ComputeInnerSumThreadV2(); //C++ style

    class Iterator; //TODO  implement it!!!
    //out
    Iterator Begin() //TODO  implement it!!!
    {
        return  Iterator{Data, 0};
    }
    Iterator End() //To-do
    {
        return Iterator{Data, Rows*Columns};
    }
    class Iterator //inner class
    {
    public:
        Iterator(Matrix::MatrixType& data, unsigned index):
            Data{data},
            CurrentIndex{index}
        {};
        DataType& operator*()
        {
            //convert array index to matrix index type
            unsigned row = CurrentIndex / Data[0].size();
            unsigned col = CurrentIndex % Data[0].size();
            return Data[row][col];
        }
        const DataType& operator*() const
        {

            //convert array index to matrix index type
            unsigned row = CurrentIndex / Data[0].size();
            unsigned col = CurrentIndex % Data[0].size();
            return Data[row][col];
        }
    private:
        Matrix::MatrixType& Data;
        unsigned CurrentIndex{};

    };
};

template <typename DataType>
Matrix<DataType>::Matrix():
    Data{MatrixType(0, RowType(0, DataType{}))},
    Rows{0},
    Columns{0}
{}

template<typename DataType>
Matrix<DataType>::Matrix(const std::size_t& rows, const std::size_t& columns):
    Data{ MatrixType(rows, RowType(columns, DataType{} ))},
    Rows{rows},
    Columns{columns}
{
    std::cout<<"explicit"<<"\n";
}

template<typename DataType>
DataType Matrix<DataType>::ComputeInnerSumThreadV2()
{
    auto start = std::chrono::steady_clock::now();
    std::vector<std::shared_future<DataType>> rowSums(Rows);
    auto accumulate = std::accumulate<typename RowType::const_iterator, DataType>;
    for(unsigned rowIndex=0; rowIndex< Rows; ++rowIndex)
    {
        rowSums[rowIndex] = std::async(accumulate, Data[rowIndex].begin(),Data[rowIndex].end(), DataType{});
    }

    DataType totalSum{};
    for(auto rowSum: rowSums)
    {
        totalSum += rowSum.get();
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> seconds = end - start;
    std::cout << "time: " << seconds.count() << "s\n";
    return totalSum;
}

template<typename DataType>
DataType Matrix<DataType>::ComputeInnerSumThread()
{

    auto start = std::chrono::steady_clock::now();
    auto accumulate = [this](unsigned rowIndex, DataType& sum)
    {
        sum = std::accumulate(Data[rowIndex].begin(), Data[rowIndex].end(), DataType{});
    };

    std::vector<std::thread> threads;
    std::vector<DataType> rowSums(Rows, DataType{});

    //creating
    for(unsigned rowIndex=0; rowIndex<Rows; ++rowIndex)
    {
        threads.emplace_back(std::thread(accumulate, rowIndex , std::ref(rowSums[rowIndex])));
    }

    //exect
    for(auto& thread: threads)
    {
        thread.join();
    }
    DataType totalSum = std::accumulate( rowSums.begin(), rowSums.end(), DataType{});

    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> seconds = end - start;
    std::cout << "time: " << seconds.count() << "s\n";
    return totalSum;
}

template<typename DataType>
DataType Matrix<DataType>::ComputeInnerSum()
{
    auto start = std::chrono::steady_clock::now();
    DataType sum{} ;
    for(auto row:Data)
    {
        sum +=  std::accumulate(row.begin(), row.end(), DataType{} );
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> seconds = end - start;
    std::cout << "time: " << seconds.count() << "s\n";

    return sum;
}

}

int main()
{

    myThread::Matrix<int>m1{4,4};
    myThread::Matrix<int>m2{4,4};
    m1[0][0]=1;
    m1[0][1]=1;
    m1[0][2]=1;
    m1[0][3]=1;

    m2[0][0]=1;
    m2[1][0]=1;
    m2[2][0]=1;
    m2[3][0]=1;

    std::cout<<(m1*m2);
}

