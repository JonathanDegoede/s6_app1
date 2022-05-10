#include <tuple>
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <future>

class Delimiter{
    private:
        int begin; //Included
        int end; //Excluded
        int length=0; //Size between begin and end

    public:

        Delimiter(){}

        Delimiter(int begin, int end){

            if(end <= begin){
                throw std::invalid_argument("Delimiter: End must be > than begin");
            }

            this->begin = begin;
            this->end = end;
            this->length = end-begin;
        }

        int getBegin(){
            return this->begin;
        }

        int getEnd(){
            return this->end;
        }

        int getLength(){
            return this->length;
        }

};

typedef std::vector<float> Dim1;
typedef std::vector<Dim1> Dim2;
typedef std::vector<Dim2> Dim3;
typedef std::vector<Dim3> Matrix4D;
typedef std::tuple<int, int, int, int> PosTuple;
typedef std::tuple<Matrix4D, Matrix4D> TimestepRes;
typedef std::vector<Delimiter> DelimiterVec;

void print_3d(Dim3 mat){
    std::cout << "Result matrix 3d" << std::endl;

    int i=0;
    int j=0;
    int k=0;
    int count=0;
    for(auto row : mat){
        for(auto col: row){
            for(auto depth: col){
               std::cout << "pos : (" << k << "," << j << "," << i << ") is : " << depth << std::endl;
               i++;
               count++;
            }
            i=0;
            j++;
        }
        i=0;
        j=0;
        k++;
    }
    std::cout << "number of vals in matrix : " << count << std::endl;
}

void print_4d(Matrix4D mat, int size, int num_field_components){
    std::cout << "Result matrix 4d" << std::endl;

    int count = 0;
    for(int field_comp=0; field_comp<num_field_components; field_comp++){
        for(int z=0; z<size; z++){
            for(int y=0; y<size; y++){
                for(int x=0; x < size; x++){
                    std::cout << "pos : (" << x << "," << y << "," << z << "," << field_comp << ") is : " << mat[x][y][z][field_comp] << std::endl;
                    count++;
                }
            }
        }
        std::cout << std::endl;
    }
    std::cout << "number of vals in matrix : " << count << std::endl;
}

void print_4d_to_file(std::string filename, Matrix4D mat, int size, int num_field_components){
    std::ofstream file;
    file.open(filename);
    for(int field_comp=0; field_comp<num_field_components; field_comp++){
        for(int z=0; z<size; z++){
            for(int y=0; y<size; y++){
                for(int x=0; x < size; x++){
                    file << "(" << x << ", "<< y << ", "<< z << ", "<< field_comp << "): ";
                    file << std::fixed << std::setprecision(5) << mat[x][y][z][field_comp] << std::endl;
                }
            }
        }
        file << std::endl;
        file << std::endl;
    }
    file.close();
}

Matrix4D gen_mat4d(int size, int num_field_components){
    Matrix4D my_matrix(size, Dim3(size, Dim2(size, Dim1(num_field_components))));
    int count = 1;
    for(int field_comp=0; field_comp<num_field_components; field_comp++){
        for(int z=0; z<size; z++){
            for(int y=0; y<size; y++){
                for(int x=0; x < size; x++){
                    my_matrix[x][y][z][field_comp] = count++;
                }
            }
        }
    }

    return my_matrix;
}

struct SourceResult{
    PosTuple src_pos;
    float src_val;
};

class WaveEquation{
    private:
        float courant_number;
        int size;
        int field_components;
        int index;
        Matrix4D E;
        Matrix4D H;

    public:
        WaveEquation(float courant_number, int size, int field_components){
            Matrix4D my_matrix(size, Dim3(size, Dim2(size, Dim1(field_components))));
            this->E = my_matrix;
            this->H = my_matrix;
            this->size = size;
            this->field_components = field_components;
            this->courant_number = courant_number;
            this->index = 0;
        }

        Matrix4D get_E(){
            return this->E;
        }

        Matrix4D get_H(){
            return this->H;
        }

        void operator()(int field_component, int slice, int slice_index){
            Dim3 field; //This will be plotted
            Matrix4D input_field;
            Delimiter x_del;
            Delimiter y_del;
            Delimiter z_del;

            if(field_component < 3){
                input_field = this->E;
            }
            else{
                input_field = this->H;
                field_component = field_component % 3;
            }
            if(slice == 0){
                x_del = Delimiter(slice_index, slice_index+1);
                y_del = Delimiter(0, this->size);
                z_del = Delimiter(0, this->size);
            }
            else if(slice == 1){
                x_del = Delimiter(0, this->size);
                y_del = Delimiter(slice_index, slice_index+1);
                z_del = Delimiter(0, this->size);
            }
            else if(slice == 2){
                x_del = Delimiter(0, this->size);
                y_del = Delimiter(0, this->size);
                z_del = Delimiter(slice_index, slice_index+1);
            }
            std::cout << "i will slice input field" << std::endl;
            field = this->slice(input_field, x_del, y_del, z_del, field_component); //A valider
            SourceResult src_res = source(this->index);
            TimestepRes E_H = this->timestep(this->E, this->H, this->courant_number, src_res.src_pos, src_res.src_val);
            this->E = std::get<0>(E_H);
            this->H = std::get<1>(E_H);
            this->index = this->index+1;
        }

        Matrix4D add_to_4d(Matrix4D mat_4d, Dim3 mat_3d,  Delimiter x_del, Delimiter y_del, Delimiter z_del, int field_component){
            int i = 0;
            int j = 0;
            int k = 0;

            for(int x = x_del.getBegin(); x < x_del.getEnd(); x++){
                for(int y = y_del.getBegin(); y < y_del.getEnd(); y++){
                    for(int z = z_del.getBegin(); z < z_del.getEnd(); z++){

                        mat_4d[x][y][z][field_component] += mat_3d[k][j][i];
                        i++;
                    }
                    i = 0;
                    j++;
                }
                i = 0;
                j = 0;
                k++;
            }

            return mat_4d;
        }

        Matrix4D sub_from_4d(Matrix4D mat_4d, Dim3 mat_3d,  Delimiter x_del, Delimiter y_del, Delimiter z_del, int field_component){
            int i = 0;
            int j = 0;
            int k = 0;
            for(int x = x_del.getBegin(); x < x_del.getEnd(); x++){
                for(int y = y_del.getBegin(); y < y_del.getEnd(); y++){
                    for(int z = z_del.getBegin(); z < z_del.getEnd(); z++){                        
                        mat_4d[x][y][z][field_component] -= mat_3d[k][j][i];
                        i++;
                    }
                    i = 0;
                    j++;
                }
                i = 0;
                j = 0;
                k++;
            }

            return mat_4d;
        }

        Dim3 sub_3d(Dim3 mat1, Dim3 mat2){
            int i=0;
            int j=0;
            int k=0;
            for (auto depth = mat1.begin(); depth != mat1.end(); depth++) {
                for (auto row = depth->begin(); row != depth->end(); row++) {
                    for(auto col = row->begin(); col != row->end(); col++){
                        mat1[k][j][i] = mat1[k][j][i] - mat2[k][j][i];
                        i++;
                    }
                    i=0;
                    j++;
                }
                i=0;
                j=0;
                k++;
            }

            return mat1;
        }

        Matrix4D add_4d_to_4d(Matrix4D mat1, Matrix4D mat2, int size, int num_field_components, float multiplicator){
            for(int field_comp=0; field_comp<num_field_components; field_comp++){
                for(int z=0; z<size; z++){
                    for(int y=0; y<size; y++){
                        for(int x=0; x < size; x++){
                            mat1[x][y][z][field_comp] += multiplicator*mat2[x][y][z][field_comp];
                        }
                    }
                }
            }

            return mat1;
        }

        Matrix4D sub_4d_from_4d(Matrix4D mat1, Matrix4D mat2, int size, int num_field_components, float multiplicator){
            for(int field_comp=0; field_comp<num_field_components; field_comp++){
                for(int z=0; z<size; z++){
                    for(int y=0; y<size; y++){
                        for(int x=0; x < size; x++){
                            mat1[x][y][z][field_comp] -= multiplicator*mat2[x][y][z][field_comp];
                        }
                    }
                }
            }

            return mat1;
        }

        Matrix4D add_to_4d_at_pos(Matrix4D mat, PosTuple pos, float val, int size, int num_field_components){
            int x = std::get<0>(pos);
            int y = std::get<1>(pos);
            int z = std::get<2>(pos);
            int field = std::get<3>(pos);
            mat[x][y][z][field] += val;
            return mat;
        }

        Dim3 slice(Matrix4D input_mat, Delimiter x_del, Delimiter y_del, Delimiter z_del, int field_component){
            Dim3 result_mat = Dim3(x_del.getLength(), Dim2(y_del.getLength(), Dim1(z_del.getLength())));

            int i = 0;
            int j = 0;
            int k = 0;
            for(int x = x_del.getBegin(); x < x_del.getEnd(); x++){
                for(int y = y_del.getBegin(); y < y_del.getEnd(); y++){
                    for(int z = z_del.getBegin(); z < z_del.getEnd(); z++){
                        
                        result_mat[k][j][i] = input_mat[x][y][z][field_component];
                        i++;
                    }
                    i = 0;
                    j++;
                }
                i = 0;
                j= 0;
                k++;
            }

            return result_mat;
        }

        Dim3 sub_3d_multi(Matrix4D mat, DelimiterVec del1, DelimiterVec del2, int field){

            std::future<Dim3> mat1_future = std::async(std::launch::async, [&]{ return this->slice(mat, del1[0], del1[1], del1[2], field);});
            std::future<Dim3> mat2_future = std::async(std::launch::async, [&]{return this->slice(mat, del2[0], del2[1], del2[2], field);});

            Dim3 mat1 = mat1_future.get();
            Dim3 mat2 = mat2_future.get();

            return this->sub_3d(mat1, mat2);
        }

        Matrix4D curl_E(Matrix4D E){
            Matrix4D curl_E(this->size, Dim3(this->size, Dim2(this->size, Dim1(this->field_components))));
            Delimiter all = Delimiter(0, size);
            Delimiter not_first = Delimiter(1, size);
            Delimiter not_last = Delimiter(0, size-1);

            std::future<Dim3> mat1_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(E, {all, not_first, all}, {all, not_last, all}, 2);});
            std::future<Dim3> mat2_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(E, {all, all, not_first}, {all, all, not_last}, 1);});
            std::future<Dim3> mat3_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(E, {all, all, not_first}, {all, all, not_last}, 0);}); 
            std::future<Dim3> mat4_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(E, {not_first, all, all}, {not_last, all, all}, 2);});
            std::future<Dim3> mat5_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(E, {not_first, all, all}, {not_last, all, all}, 1);});
            std::future<Dim3> mat6_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(E, {all, not_first, all}, {all, not_last, all}, 0);});

            curl_E = add_to_4d(curl_E, mat1_future.get(), all, not_last, all, 0);
            curl_E = sub_from_4d(curl_E, mat2_future.get(), all, all, not_last, 0);
            curl_E = add_to_4d(curl_E, mat3_future.get(), all, all, not_last, 1);
            curl_E = sub_from_4d(curl_E, mat4_future.get(), not_last, all, all, 1);
            curl_E = add_to_4d(curl_E, mat5_future.get(), not_last, all, all, 2);
            curl_E = sub_from_4d(curl_E, mat6_future.get(), all, not_last, all, 2);

            return curl_E;
        }

        Matrix4D curl_H(Matrix4D H){
            Matrix4D curl_H(this->size, Dim3(this->size, Dim2(this->size, Dim1(this->field_components))));
            Delimiter all = Delimiter(0, size);
            Delimiter not_first = Delimiter(1, size);
            Delimiter not_last = Delimiter(0, size-1);

            std::future<Dim3> mat1_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(H, {all, not_first, all}, {all, not_last, all}, 2);});
            std::future<Dim3> mat2_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(H, {all, all, not_first}, {all, all, not_last}, 1);});
            std::future<Dim3> mat3_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(H, {all, all, not_first}, {all, all, not_last}, 0);}); 
            std::future<Dim3> mat4_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(H, {not_first, all, all}, {not_last, all, all}, 2);});
            std::future<Dim3> mat5_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(H, {not_first, all, all}, {not_last, all, all}, 1);});
            std::future<Dim3> mat6_future = std::async(std::launch::async, [&]{return this->sub_3d_multi(H, {all, not_first, all}, {all, not_last, all}, 0);});

            curl_H = add_to_4d(curl_H, mat1_future.get(), all, not_first, all, 0);
            curl_H = sub_from_4d(curl_H, mat2_future.get(), all, all, not_first, 0);
            curl_H = add_to_4d(curl_H, mat3_future.get(), all, all, not_first, 1);
            curl_H = sub_from_4d(curl_H, mat4_future.get(), not_first, all, all, 1);
            curl_H = add_to_4d(curl_H, mat5_future.get(), not_first, all, all, 2);
            curl_H = sub_from_4d(curl_H, mat6_future.get(), all, not_first, all, 2);

            return curl_H;
        }

        SourceResult source(int index){
            SourceResult src_result;
            src_result.src_pos = std::make_tuple(floor(this->size/3),floor(this->size/3),floor(this->size/2),0);
            src_result.src_val = 0.1*sin(0.1*index);
            std::cout<<"im in source and index is : " << index << " .  0.1*sin(0.1*index) evaluates to : " << src_result.src_val << std::endl;

            return src_result;
        }

        TimestepRes timestep(Matrix4D E, Matrix4D H, float courant_number, PosTuple source_pos, float source_val){

            std::cout << "im in timestep" << std::endl;
            Matrix4D curl_H = this->curl_H(H);
            std::cout << "done with curl H" << std::endl;

            E = add_4d_to_4d(E, curl_H, this->size, this->field_components, courant_number);
            std::cout << "done with adding curl H to E" << std::endl;

            E = add_to_4d_at_pos(E, source_pos, source_val, this->size, this->field_components);

            Matrix4D curl_E = this->curl_E(E);
            std::cout << "done with curl E" << std::endl;

            H = sub_4d_from_4d(H, curl_E, this->size, this->field_components, courant_number);
            std::cout << "done with subbing curl E from H " << std::endl;

            std::cout << "Done with timestep, returning" << std::endl;

            return std::make_tuple(E, H);
        }
};

void testCurl(){
    WaveEquation w = WaveEquation(0.1, 3, 3);

    auto mat = gen_mat4d(3,3);
    print_4d(mat, 3, 3);

    auto res_curl_E = w.curl_E(mat);
    print_4d(res_curl_E, 3, 3);

    auto res_curl_H = w.curl_H(mat);
    print_4d(res_curl_H, 3, 3);
}

void testAdd4dto4d(){
    WaveEquation w = WaveEquation(0.1, 3, 3);

    auto mat = gen_mat4d(3,3);
    auto mat2 = gen_mat4d(3,3);

    auto mat_add = w.sub_4d_from_4d(mat, mat2, 3, 3, 1);
    print_4d(mat_add, 3, 3);
}

void testSub4dFrom4d(){
    WaveEquation w = WaveEquation(0.1, 3, 3);

    auto mat = gen_mat4d(3,3);
    auto mat2 = gen_mat4d(3,3);

    auto mat_add = w.sub_4d_from_4d(mat, mat2, 3, 3, 1);
    print_4d(mat_add, 3, 3);
}


void testAddto4dAtPos(){
    WaveEquation w = WaveEquation(0.1, 3, 3);

    auto mat = gen_mat4d(3,3);
    PosTuple pos = std::make_tuple(0,0,0,0);

    auto mat_add = w.add_to_4d_at_pos(mat, pos, 1, 3, 3);
    print_4d(mat_add, 3, 3);
}

void testTimeStep(){
    WaveEquation w = WaveEquation(0.1, 3, 3);

    auto mat = gen_mat4d(3,3);
    auto mat2 = gen_mat4d(3,3);

    TimestepRes E_H;

    int index = 0;

    for(int i=0; i<1000; i++){
        SourceResult src_res = w.source(index++); 
        std::cout << "src_pos: " << std::get<0>(src_res.src_pos) << "," <<  std::get<1>(src_res.src_pos) << "," <<  
        std::get<2>(src_res.src_pos)  << "," <<  std::get<3>(src_res.src_pos) << " src_val: " << src_res.src_val << std::endl;

        E_H = w.timestep(mat, mat2, 0.1, src_res.src_pos, src_res.src_val);
        mat = std::get<0>(E_H);
        mat2 = std::get<1>(E_H);
    }

    std::cout<< "E after 10  timestep" << std::endl; 
    print_4d(mat, 3, 3);
}


int main(int argc, char const *argv[])
{

    // Convention : [x][y][z][field]
    // field represents a cube index from 0 to field_components (excluded)

    // The cube (nxnxn) :
                            // col = x
                     //  [19 20 21]
          //  [10 11 12] [22 23 24]
    //[1 2 3] [13 14 15] [25 26 27] // row = y
    //[4 5 6] [16 17 18]  
    //[7 8 9]  
     // depth = z 


    int n = 5;
    float courant_number = 0.1;
    int field_components = 3;

    //For output
    std::string folder_path = "./output_waveprop/cpp/";

    WaveEquation w = WaveEquation(courant_number, n, field_components);
    for(int i=0; i<100; i++){
        w(0, 0, 0);
    }

    Matrix4D E = w.get_E();
    Matrix4D H = w.get_H();

    print_4d_to_file(folder_path + "cpp_4d_E_res_multi.txt", E, n, field_components);
    print_4d_to_file(folder_path + "cpp_4d_H_res_multi.txt", H, n, field_components);

    return 0;
}
