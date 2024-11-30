
# for i in range(20, 40):
#     print(f"{{{i*4 + 0}, {i*4 + 1}, {i*4 + 2}, {i*4 + 3}}},")

# for i in range(56, 80):
#     print(f"{i}, ", end="")


# for i in range(1,40):
#     print(f"case {i}:");
#     print(f"    led{i} = 0;");
#     print(f"    Delay500ms();");
     
    
#  generate tail
i = 56
while(i<80):
    i += 3
    print(f"{i-1}, {i}, ", end="")