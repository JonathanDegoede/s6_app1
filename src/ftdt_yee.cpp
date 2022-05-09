#include <tuple>
#include <math.h>
#include <vector>
#include <iostream>

typedef std::vector<int> Dim1;
typedef std::vector<Dim1> Dim2;
typedef std::vector<Dim2> Dim3;
typedef std::vector<Dim3> Matrix4D;

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

class Delimiter{
    private:
        int begin; //Included
        int end; //Excluded
        int length=0; //Size between begin and end

    public:

        Delimiter(){}

        Delimiter(int end){
            this->begin = end;
            this->end = end+1;
            this->length = end-begin;
        }

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

struct SourceResult{
    std::tuple<int, int, int, int> src_tuple;
    float sin_result;
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

            std::cout << "I created a WaveEquation class" << std::endl;
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
                x_del = Delimiter(slice_index);
                y_del = Delimiter(this->size, true);
                z_del = Delimiter(this->size, true);
                //field = field[slice_index, :, :, field_component] //TODO
            }
            else if(slice == 1){
                x_del = Delimiter(this->size, true);
                y_del = Delimiter(slice_index);
                z_del = Delimiter(this->size);
                // field = field[:, slice_index, :, field_component] //TODO
            }
            else if(slice == 2){
                x_del = Delimiter(this->size, true);
                y_del = Delimiter(this->size, true);
                z_del = Delimiter(slice_index);
                // field = field[:, :, slice_index, field_component] //TODO
            }
            field = this->slice(input_field, x_del, y_del, z_del, field_component);
            SourceResult src_res = source(this->index);
            // self.E, self.H = timestep(self.E, self.H, self.courant_number, source_pos, source_index); //TODO
            this->index += 1;
        }

        Matrix4D add_to_4d(Matrix4D mat_4d, Dim3 mat_3d,  Delimiter x_del, Delimiter y_del, Delimiter z_del, int field_component){
            int i = 0;
            int j = 0;
            int k = 0;

            for(int x = x_del.getBegin(); x < x_del.getEnd(); x++){
                for(int y = y_del.getBegin(); y < y_del.getEnd(); y++){
                    for(int z = z_del.getBegin(); z < z_del.getEnd(); z++){

                        // std::cout << "mat_4d current value at pos : (" << field_component << "," << z << "," << y << "," << x << ") is : "
                        // << mat_4d[field_component][z][y][x]
                        // << std::endl;

                        // std::cout << "mat_3d current value at pos : (" << field_component << "," << k << "," << j << "," << i << ") is : "
                        // << mat_3d[k][j][i]
                        // << std::endl;
                        
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

            // std::cout << "result of add to 4d" << std::endl;
            // print_4d(mat_4d, this->size, this->field_components);

            return mat_4d;
        }

        Matrix4D sub_from_4d(Matrix4D mat_4d, Dim3 mat_3d,  Delimiter x_del, Delimiter y_del, Delimiter z_del, int field_component){
            int i = 0;
            int j = 0;
            int k = 0;
            for(int x = x_del.getBegin(); x < x_del.getEnd(); x++){
                for(int y = y_del.getBegin(); y < y_del.getEnd(); y++){
                    for(int z = z_del.getBegin(); z < z_del.getEnd(); z++){

                        // std::cout << "mat_4d current value at pos : (" << field_component << "," << z << "," << y << "," << x << ") is : "
                        // << mat_4d[field_component][z][y][x]
                        // << std::endl;

                        // std::cout << "mat_3d current value at pos : (" << k << "," << j << "," << i << ") is : "
                        // << mat_3d[k][j][i]
                        // << std::endl;
                        
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

            // std::cout << "result of sub from 4d" << std::endl;
            // print_4d(mat_4d);

            return mat_4d;
        }

        Dim3 sub_3d(Dim3 mat1, Dim3 mat2){
            int i=0;
            int j=0;
            int k=0;
            // std:: cout << "sub 3d :" << std::endl;
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

            // std::cout << "result of sub 3d" << std::endl;
            // print_3d(mat1);

            return mat1;
        }

        Dim3 slice(Matrix4D input_mat, Delimiter x_del, Delimiter y_del, Delimiter z_del, int field_component){
            Dim3 result_mat = Dim3(x_del.getLength(), Dim2(y_del.getLength(), Dim1(z_del.getLength())));

            // std::cout << "look mom im in slice!" << std::endl;
            // std::cout << "using syntax (x,y,z,field)" << std::endl;

            int i = 0;
            int j = 0;
            int k = 0;
            for(int x = x_del.getBegin(); x < x_del.getEnd(); x++){
                for(int y = y_del.getBegin(); y < y_del.getEnd(); y++){
                    for(int z = z_del.getBegin(); z < z_del.getEnd(); z++){

                        // std::cout << "current value at pos : (" << field_component << "," << z << "," << y << "," << x << ") is : "
                        // << input_mat[field_component][z][y][x]
                        // << std::endl;
                        
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

            // std::cout << "result of slicing" << std::endl;
            // print_3d(result_mat);

            return result_mat;
        }

        Matrix4D curl_E(Matrix4D E){
            //Valider que les delimiteurs sont bons (x,y,z) vs le code python
            Matrix4D curl_E(this->size, Dim3(this->size, Dim2(this->size, Dim1(this->field_components))));
            Delimiter all = Delimiter(0, size);
            Delimiter not_first = Delimiter(1, size);
            Delimiter not_last = Delimiter(0, size-1);

            auto add3d_res = sub_3d(slice(E, all, not_first, all, 2), slice(E, all, not_last, all, 2));
            curl_E = add_to_4d(curl_E, add3d_res, all, not_last, all, 0);

            add3d_res = sub_3d(slice(E, all, all, not_first, 1), slice(E, all, all, not_last, 1));
            curl_E = sub_from_4d(curl_E, add3d_res, all, all, not_last, 0);

            add3d_res = sub_3d(slice(E, all, all, not_first, 0), slice(E, all, all, not_last, 0));
            curl_E = add_to_4d(curl_E, add3d_res, all, all, not_last, 1);

            add3d_res = sub_3d(slice(E, not_first, all, all, 2), slice(E, not_last, all, all, 2));
            curl_E = sub_from_4d(curl_E, add3d_res, not_last, all, all, 1);

            add3d_res = sub_3d(slice(E, not_first, all, all, 1), slice(E, not_last, all, all, 1));
            curl_E = add_to_4d(curl_E, add3d_res, not_last, all, all, 2);

            add3d_res = sub_3d(slice(E, all, not_first, all, 0), slice(E, all, not_last, all, 0));
            curl_E = sub_from_4d(curl_E, add3d_res, all, not_last, all, 2);

            return curl_E;
        }

        SourceResult source(int index){
            SourceResult src_result;
            src_result.src_tuple = std::make_tuple(floor(this->size/3),floor(this->size/3),floor(this->size/2),0);
            src_result.sin_result = 0.1*sin(0.1*index);
            return src_result;
        }

        void timestep(){

        }
};

void testGenerator(){

    std::cout << "im in testGenerator" << std::endl;
    int size = 2;
    int field_components = 3;
    int courant_number = 0.1;

    WaveEquation w = WaveEquation(courant_number, size, field_components);

    auto mat = gen_mat4d(size, 3);
    std::cout << "input arr" << std::endl;
    // print_4d(mat);

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



    Delimiter x_del = Delimiter(0,3);
    Delimiter y_del = Delimiter(0,2);
    Delimiter z_del = Delimiter(0,3);
    int cube_num = 0;

    // auto res3d = w.slice(e,x_del, y_del, z_del, cube_num);

    // auto res3d_add = w.add_3d(res3d, res3d);

    // print_3d(res3d);

    // auto res4d = w.sub_from_4d(e, res3d, x_del, y_del, z_del, 0);

    // print_4d(w.curl_E(mat));
}

void testCurl(){
    WaveEquation w = WaveEquation(0.1, 3, 3);

    auto mat = gen_mat4d(3,3);
    print_4d(mat, 3, 3);

    auto res = w.curl_E(mat);
    print_4d(res, 3, 3);
}


int main(int argc, char const *argv[])
{
    int n = 3;
    float courant_number = 0.1;
    int field_components = 3;

    //Not clear what those are for
    // float r = 0.01;
    // int = 30;

    // WaveEquation w = WaveEquation(courant_number, n);

    testCurl();

    /* code */
    return 0;
}
