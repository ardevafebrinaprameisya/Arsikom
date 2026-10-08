#include <stdio.h>
#include <stdint.h>

#define RAM_SIZE 16

#define LDA 0x0 //0H
#define ADD 0x1 //1H
#define SUB 0x2 //2H
#define OUT 0xE //EH
#define HLT 0xF //FH

typedef enum { t1, t2, t3, t4, t5, t6 } state_t;

/* Register */
uint8_t PC = 0; // menyimpan alamat instruksi berikutnya
uint8_t MAR = 0; //menyimpan alamat RAM yang sedang diakses
uint8_t IR = 0; // menyimpan instruksi yang sedang diproses
uint8_t A = 0; // register utama untuk operasi aritmatika
uint8_t B = 0; // menyimpan operand
uint8_t OUT_REG = 0; //menyimpan nilai yang akan dikeluarkan sebagai output


/* Active High  sinyal aktif = 1 */
int Cp = 0, Ep = 0, EA = 0, EU = 0, SU = 0; //Counter program, Enable Program Counter, Enable Accumulator, Enable ALU, Subtract

/* Active Low  sinyal aktif = 0 */
int LM = 1, CE = 1, LI = 1, EI = 1; // Load Memory Adress Register, Count/Memory Enable, Load Instruction Register, Enable Instruction Register
int LA = 1, LB = 1, LO = 1; // Load Accumulator, Load B register, Load Output Register

uint8_t RAM[RAM_SIZE];
int program_running = 1;

/* Reset semua control signal */
void reset_control_signal()
{
    Cp = Ep = EA = EU = SU = 0; //kondisi tidak aktif
    LM = CE = LI = EI = LA = LB = LO = 1; // kondisi tidak aktif
}


/* Tampilkan register */
void tampilkanRegister()
{
    printf("PC=%02X MAR=%02X IR=%02X A=%02X B=%02X OUT=%02X\n",
           PC, MAR, IR, A, B, OUT_REG);
}


/* Tampilkan control signal yang aktif */
void tampilkanSinyal()
{
    printf("Signal: ");

    if (Cp) printf("Cp "); // mengecek apakah nilai sinyal aktif (1) atau tidak 
    if (Ep) printf("Ep ");
    if (EA) printf("EA ");
    if (EU) printf("EU ");
    if (SU) printf("SU ");

    if (!LM) printf("LM "); // mengecek apakah nilai sinyal aktif (0) atau tidak
    if (!CE) printf("CE ");
    if (!LI) printf("LI ");
    if (!EI) printf("EI ");
    if (!LA) printf("LA ");
    if (!LB) printf("LB ");
    if (!LO) printf("LO ");

    printf("\n");
}


/* fungsi untuk menampilkan binary */
void tampilkanBinary(uint8_t nilai)
{
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (nilai >> i) & 1);
    }
}


/* State Machine */
state_t state_machine(state_t current_state) // melihat keadaan saat ini dan menentukan langkah selanjutnya berdasarkan instruksi yang ada di IR
{
    state_t next_state = current_state;

    switch (current_state)
    {
        /* FETCH */

        case t1:
            reset_control_signal(); // mengembalikan semua control signal ke kondisi tidak aktif

            next_state = t2;
            Ep = 1; // mengeluarkan nilai yang ada pada pc
            LM = 0; // memasukkan alamat tersebut ke MAR

            MAR = PC;

            printf("\n--- T1 ---\n");
            printf("PC -> MAR\n");
            tampilkanRegister();
            tampilkanSinyal();
            break;


        case t2:
            reset_control_signal();

            next_state = t3;
            Cp = 1;

            PC++;

            printf("\n--- T2 ---\n");
            printf("PC + 1\n");
            tampilkanRegister();
            tampilkanSinyal();
            break;


        case t3:
            reset_control_signal(); // mengembalikan semua control signal ke kondisi tidak aktif

            next_state = t4;
            CE = 0; //mengambil data dari RAM berdasarkan alamat yang terdapat pada MAR
            LI = 0; //memasukkan data tersebut ke IR

            IR = RAM[MAR];

            printf("\n--- T3 ---\n");
            printf("RAM[%02X] -> IR\n", MAR);
            tampilkanRegister();
            tampilkanSinyal();

            printf("\n========== ");

            switch ((IR >> 4) & 0x0F) //mengenaili jenis instruksi dalam IR & mengambil 4 bit pertama sebagai opcode
            {
                case LDA:
                    printf("LDA %XH", IR & 0x0F); 
                    break;

                case ADD:
                    printf("ADD %XH", IR & 0x0F);
                    break;

                case SUB:
                    printf("SUB %XH", IR & 0x0F);
                    break;

                case OUT:
                    printf("OUT");
                    break;

                case HLT:
                    printf("HLT");
                    break;
            }

            printf(" ==========\n");
            break;


        /* DECODE & EXECUTE */

        case t4: // tentukan instruksi dan siapkan alamat
        {
            uint8_t opcode = (IR >> 4) & 0x0F; //mengetahui jenis instruksi & digeser kekanan sebanyak 4 bit
            uint8_t alamat = IR & 0x0F; // mengetahui alamat data & mengambil 4 bit terakhir dari IR

            reset_control_signal(); // mengembalikan semua control signal ke kondisi tidak aktif

            if (opcode == LDA) // mengambil alamat 9 ir-> MAR & MAR menunjuk ke alamat data yang akan diambil pada T5.”
            {
                EI = 0;
                LM = 0;
                MAR = alamat; // IR  memberikan alamat -> MAR

                printf("\n--- T4 ---\n");
                printf("LDA: IR -> MAR\n");
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t5; // mengambil data dari RAM ke register A
            }

            else if (opcode == ADD)
            {
                EI = 0;
                LM = 0;
                MAR = alamat; // IR  memberikan alamat -> MAR

                printf("\n--- T4 ---\n");
                printf("ADD: IR -> MAR\n");
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t5; // mengambil data dari RAM ke register B
            }

            else if (opcode == SUB)
            {
                EI = 0;
                LM = 0;
                MAR = alamat; // IR  memberikan alamat -> MAR

                printf("\n--- T4 ---\n");
                printf("SUB: IR -> MAR\n");
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t5; // mengambil data dari RAM ke register B
            }

            else if (opcode == OUT)
            {
                EA = 1;
                LO = 0;

                OUT_REG = A; // nilai A -> OUT_REG

                printf("\n--- T4 ---\n");
                printf("OUT: A -> OUT_REG\n");
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t1; //diarahkan kembali ke T1 untuk mengambil instruksi berikutnya
            }

            else if (opcode == HLT)
            {
                printf("\n--- T4 ---\n");
                printf("HLT: Program berhenti\n");

                program_running = 0;
                next_state = t4;
            }

            else
            {
                printf("Opcode tidak dikenal\n");
                program_running = 0;
            }

            break;
        }


        case t5: //ambil data dari RAM ke register A atau B
        {
            uint8_t opcode = (IR >> 4) & 0x0F; //mengetahui jenis instruksi & digeser kekanan sebanyak 4 bit

            reset_control_signal(); // mengembalikan semua control signal ke kondisi tidak aktif

            if (opcode == LDA)
            {
                CE = 0;
                LA = 0;

                A = RAM[MAR]; //data dari MAR diambil dari RAM -> A

                printf("\n--- T5 ---\n");
                printf("RAM[%02X] -> A\n", MAR); //mengambil RAM[09] nilai 02 sehingga A ->02
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t1;
            }

            else if (opcode == ADD)
            {
                CE = 0;
                LB = 0;

                B = RAM[MAR]; //data dari MAR diambil dari RAM -> B

                printf("\n--- T5 ---\n");
                printf("RAM[%02X] -> B\n", MAR); //mengambil RAM[09] nilai 02 sehingga B ->02
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t6;
            }

            else if (opcode == SUB)
            {
                CE = 0;
                LB = 0;

                B = RAM[MAR]; //data dari MAR diambil dari RAM -> B

                printf("\n--- T5 ---\n");
                printf("RAM[%02X] -> B\n", MAR); //mengambil RAM[09] nilai 02 sehingga B ->02
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t6;
            }

            break;
        }


        case t6: // melakukan operasi aritmatika
        {
            uint8_t opcode = (IR >> 4) & 0x0F; //mengetahui jenis instruksi & digeser kekanan sebanyak 4 bit

            reset_control_signal(); // mengembalikan semua control signal ke kondisi tidak aktif

            if (opcode == ADD)
            {
                EU = 1;
                LA = 0;

                A = A + B;

                printf("\n--- T6 ---\n");
                printf("A + B -> A\n");
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t1;
            }

            else if (opcode == SUB)
            {
                EU = 1;
                SU = 1;
                LA = 0;

                A = A - B;

                printf("\n--- T6 ---\n");
                printf("A - B -> A\n");
                tampilkanRegister();
                tampilkanSinyal();

                next_state = t1;
            }

            break;
        }
    }

    return next_state;
}


/* State awal */
state_t initial_state = t1; 


/* Menjalankan state machine */
void simulate_state(state_t state)
{
    while (program_running) 
        state = state_machine(state);
}


int main() // bagian utama menjalankan simulasi sap 1
{
    /* PROGRAM */
    RAM[0x0] = 0x09;     // LDA 9H
    RAM[0x1] = 0x1A;     // ADD AH
    RAM[0x2] = 0x1B;     // ADD BH
    RAM[0x3] = 0x2C;     // SUB CH
    RAM[0x4] = 0xE0;     // OUT
    RAM[0x5] = 0xF0;     // HLT

    /* DATA */
    RAM[0x9] = 0x02;
    RAM[0xA] = 0x02;
    RAM[0xB] = 0x03;
    RAM[0xC] = 0x04;

    printf("================================\n");
    printf("       SIMULASI SAP-1 DALAM C\n");
    printf("================================\n");

    printf("\nProgram:\n");
    printf("0H : LDA 9H\n");
    printf("1H : ADD AH\n");
    printf("2H : ADD BH\n");
    printf("3H : SUB CH\n");
    printf("4H : OUT\n");
    printf("5H : HLT\n");

    printf("\nData:\n");
    printf("9H = %02X\n", RAM[0x9]);
    printf("AH = %02X\n", RAM[0xA]);
    printf("BH = %02X\n", RAM[0xB]);
    printf("CH = %02X\n", RAM[0xC]);

    printf("\n========== PROSES EKSEKUSI ==========\n");

    state_t current_state = initial_state; // simulasi dimulai dari t1
    simulate_state(current_state);

    printf("\n========== HASIL AKHIR ==========\n");
    printf("A       = %02X\n", A);
    printf("OUT_REG = %02X\n", OUT_REG);

    /* binary */
    printf("Binary  = ");
    tampilkanBinary(OUT_REG);
    printf("\n");

    return 0;
}