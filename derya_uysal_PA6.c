#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the gate structure
typedef struct gate
{
    int type; // Gate type: input 0, output 1, AND 2, OR 3, NOT 4, flipflop 5

    char name[20];
    struct gate *input_gates[10]; // Array of pointers to input gates

    int number_of_inputs;
    int output;
    int former_out;                             // Former output value (used for flipflop gates)
    short evaluated;                            // Flag to indicate if the gate has been evaluated
    int (*characteristic_function)(int *, int); // Pointer to the characteristic function of the gate
} t_gate;

// Define the gate array structure
typedef struct gateArray
{
    t_gate *gates;   // Array of gates
    size_t capacity; // Capacity of the array
    size_t size;     // Current size of the array

} t_gate_array;

// Function to create a gate array with a given capacity
t_gate_array createGateArray(size_t capacity)
{
    t_gate_array gate_arr;
    gate_arr.gates = malloc(capacity * sizeof(t_gate));
    gate_arr.capacity = capacity;
    gate_arr.size = 0;
    return gate_arr;
}

// Function to resize a gate array
void resizeGateArray(t_gate_array *gateArray)
{
    size_t new_capacity = gateArray->capacity * 2;
    t_gate *new_gates = malloc(new_capacity * sizeof(t_gate));
    if (new_gates == NULL)
    {
        return;
    }
    for (size_t i = 0; i < gateArray->size; i++)
    {
        new_gates[i] = gateArray->gates[i];
    }
    free(gateArray->gates);
    gateArray->gates = new_gates;
    gateArray->capacity = new_capacity;
}

// Function to add a gate to a gate array
void addGate(t_gate_array *gateArray, t_gate gate)
{
    if (gateArray->size == gateArray->capacity)
    {
        resizeGateArray(gateArray);
    }
    gateArray->gates[gateArray->size] = gate;
    gateArray->size++;
}

// AND gate characteristic function
int and_function(int *input, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (!input[i])
        {
            return 0;
        }
    }
    return 1;
}

// OR gate characteristic function
int or_function(int *input, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (input[i])
        {
            return 1;
        }
    }
    return 0;
}

// NOT gate characteristic function
int not_function(int *input, int n)
{
    return !input[0];
}

// Flipflop gate characteristic function
int flipflop_function(int *input, int former_out)
{
    if (input[0] == 1)
    {
        return !former_out; // Invert the output if the input is 1
    }
    else
    {
        return former_out; // Keep the output if the input is 0
    }
}

// Function to reset the evaluated flag of all gates in a gate array
void reset_gates(t_gate_array *gates)
{
    for (size_t i = 0; i < gates->size; i++)
    {
        gates->gates[i].evaluated = 0;
    }
}

// Function to create a gate and add it to the appropriate gate array
void create_gate(t_gate_array *inputGates, t_gate_array *outputGates, t_gate_array *otherGates, char *type, char *name)
{
    t_gate new_gate;
    new_gate.number_of_inputs = 0;
    new_gate.evaluated = 0;
    new_gate.former_out = 0;
    strcpy(new_gate.name, name);

    if (strcmp(type, "INPUT") == 0)
    {
        new_gate.type = 0;
        addGate(inputGates, new_gate);
    }
    else if (strcmp(type, "OUTPUT") == 0)
    {
        new_gate.type = 1;
        addGate(outputGates, new_gate);
    }
    else
    {
        if (strcmp(type, "AND") == 0)
        {
            new_gate.type = 2;
            new_gate.characteristic_function = and_function;
        }
        if (strcmp(type, "OR") == 0)
        {
            new_gate.type = 3;
            new_gate.characteristic_function = or_function;
        }
        if (strcmp(type, "NOT") == 0)
        {
            new_gate.type = 4;
            new_gate.characteristic_function = not_function;
        }
        if (strcmp(type, "FLIPFLOP") == 0)
        {
            new_gate.type = 5;
            new_gate.characteristic_function = flipflop_function;
        }
        addGate(otherGates, new_gate);
    }
}

// Function to create a connection between two gates
void create_connection(t_gate_array *inputGates, t_gate_array *outputGates, t_gate_array *otherGates, char *from_gate, char *to_gate)
{

    t_gate *from = NULL;
    t_gate *to = NULL;

    // Find the 'from' gate in the input gates array
    for (size_t i = 0; i < inputGates->size; i++)
    {
        if (strcmp(inputGates->gates[i].name, from_gate) == 0)
        {
            from = &inputGates->gates[i];
            break;
        }
    }

    // Find the 'from' and 'to' gates in the other gates array
    for (size_t i = 0; i < otherGates->size; i++)
    {
        if (strcmp(otherGates->gates[i].name, from_gate) == 0)
        {
            from = &otherGates->gates[i];
        }
        else if (strcmp(otherGates->gates[i].name, to_gate) == 0)
        {
            to = &otherGates->gates[i];
        }
    }

    // Find the 'to' gate in the output gates array
    for (size_t i = 0; i < outputGates->size; i++)
    {
        if (strcmp(outputGates->gates[i].name, to_gate) == 0)
        {
            to = &outputGates->gates[i];
            break;
        }
    }

    if (from == NULL || to == NULL)
    {
        // printf("Connection failed %s %s\n", from_gate, to_gate);
        return;
    }
    // printf("Connection created %s %s\n", from->name, to->name);
    to->input_gates[to->number_of_inputs] = from;
    to->number_of_inputs++;
}

// Function to evaluate the output of a gate
int evaluate(t_gate *gate)
{
    if (gate->evaluated)
    {
        return gate->output;
    }

    if (gate->type == 0) // input
    {
        gate->evaluated = 1;
        return gate->output;
    }
    else if (gate->type == 1) // output
    {
        gate->evaluated = 1;
        gate->output = evaluate(gate->input_gates[0]);
        return gate->output;
    }

    int inputs[10];
    for (size_t i = 0; i < gate->number_of_inputs; i++)
    {
        inputs[i] = evaluate(gate->input_gates[i]);
    }
    if (gate->type == 5) // flipflop
    {
        gate->output = gate->characteristic_function(inputs, gate->former_out);
        gate->former_out = gate->output;
    }
    else
    {
        gate->output = gate->characteristic_function(inputs, gate->number_of_inputs);
    }

    gate->evaluated = 1;
    return gate->output;
}

int main()
{

    t_gate_array inputGates = createGateArray(10);
    t_gate_array otherGates = createGateArray(10);
    t_gate_array outputGates = createGateArray(10);

    FILE *circuit_file = fopen("circuit.txt", "r");
    char circuit_line[100];
    char str1[20], str2[20], str3[20];

    // Read the circuit file and create gates and connections
    while (fgets(circuit_line, sizeof(circuit_line), circuit_file))
    {
        if (sscanf(circuit_line, "%s %s %s", str1, str2, str3) == 3)
        {
            if (strcmp(str1, "GATE") == 0)
            {
                create_gate(&inputGates, &outputGates, &otherGates, str2, str3);
            }
            else if (strcmp(str1, "CONNECTION") == 0)
            {
                create_connection(&inputGates, &outputGates, &otherGates, str2, str3);
            }
        }
    }
    fclose(circuit_file);

    FILE *input_file = fopen("input.txt", "r");
    char input_line[100];

    // Read the input file and evaluate the circuit for each input line
    while (fgets(input_line, sizeof(input_line), input_file))
    {
        reset_gates(&inputGates);
        reset_gates(&outputGates);
        reset_gates(&otherGates);

        for (int i = 0; i < inputGates.size; i++)
        {
            inputGates.gates[i].output = input_line[i] - '0';
        }
        for (int i = 0; i < outputGates.size; i++)
        {
            printf("%d", evaluate(&outputGates.gates[i]));
        }
        printf("\n");
    }

    fclose(input_file);

    free(inputGates.gates);
    free(outputGates.gates);
    free(otherGates.gates);
}
