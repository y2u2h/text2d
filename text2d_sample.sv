//----------------------------------
// systemverilog sample program
//----------------------------------

`include "text2d_pkg.sv"
import text2d_pkg::*;

class array;
    rand byte unsigned data [];
    int unsigned width;
    int unsigned height;

    function resize(int unsigned width, int unsigned height);
        data = new[width * height];
        this.width = width;
        this.height = height;
    endfunction

    function display();
        for (int y = 0; y < height; ++y) begin
            for (int x = 0; x < width; ++x) begin
                $write("%02x ", data[width * y + x]);
            end
            $display("");
        end
    endfunction
endclass

class xshift;
    rand int unsigned val;
    constraint c_0 { val >= 0 && val <= 16; }
endclass

module text2d_sample_test;
    array arr;
    xshift shift;
    chandle wtxt;
    chandle rtxt;

    initial begin
        arr = new;
        shift = new;
        arr.resize(8, 4);

        $display("//----------------------------------");
        $display("// write phase                      ");
        $display("//----------------------------------");

        wtxt = text2d_open("./sample.txt", "w", 8, "_", 0);
        if (wtxt == null) begin
            $error(0, "text2d_open() returns null pointer !!!");
        end

        text2d_cache_resize(wtxt, 32, 32);

        for (int c = 0; c < 2; ++c) begin
            for (int y = 0; y < 32; y += 4) begin
                shift.randomize();
                for (int x = shift.val; x < 32; x += 16) begin
                    arr.randomize();
                    text2d_cache_write(wtxt, arr.data, x, y, 8, 4);
                end
            end
            text2d_cache_dump(wtxt, 1);

            if (text2d_fwrite(wtxt) < 0) begin
                $error(0, "text2d_fwrite() error !!!");
            end
            text2d_cache_clear(wtxt);
        end

        text2d_close(wtxt);

        $display("//----------------------------------");
        $display("// read phase                       ");
        $display("//----------------------------------");

        arr.resize(10, 6);

        rtxt = text2d_open("./sample.txt", "r", 8, "_", -1);
        if (rtxt == null) begin
            $error(0, "text2d_open() returns null pointer !!!");
        end

        for (int c = 0; c < 2; ++c) begin
            if (text2d_fread(rtxt, 32) < 0) begin
                $error(0, "text2d_fread() error !!!");
            end
            text2d_cache_dump(rtxt, 1);

            for (int y = 0; y < 32; y += 4) begin
                for (int x = 0; x < 32; x += 8) begin
                    text2d_cache_read(rtxt, arr.data, x - 1, y - 1, 10, 6);

                    $display("-- (x, y) = (%0d, %0d)", x, y);
                    for (int y = 0; y < 6; ++y) begin
                        for (int x = 0; x < 10; ++x) begin
                            $write("%02x ", arr.data[y * 10 + x]);
                        end
                        $display("");
                    end
                end
            end
        end

        text2d_close(rtxt);
    end
endmodule
