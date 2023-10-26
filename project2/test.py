import os, glob, subprocess

'''
Get output of hotel simulations
- can spawn each as their own process
- file output in format test-{num_guests}-{iteration}.txt

Validate output of hotel simulations


'''

FRONT = 'Front'
DESK_EMPLOYEE = 'Front desk employee'
BELLHOP = 'Bellhop'
GUEST = 'Guest'

CREATION = 0
ENTER_HOTEL = 1
REGISTER = 2
RECEIVE_KEY = 3
REQUEST_HELP = 4
BELLHOP_RECEIVE = 5
ENTER_ROOM = 6
BELLHOP_DELIVER = 7
RECEIVE_BAGS = 8
RETIRE = 9
JOIN = 10

def clean_folder(folder):
    print(f'Cleaning folder: {folder}')
    counter = 0
    for subfolder in glob.glob(f'{folder}/*'):
        for f in glob.glob(f'{subfolder}/*'):
            os.remove(f)
            counter += 1
    print(f'\t{counter} files removed')
    
def run_simulations(LO_GUESTS, HI_GUESTS, TESTS_PER_GUEST):
    print(f'Running simulation for {LO_GUESTS} to {HI_GUESTS} guests')

    output_files = []
    processes = []

    for i in range(LO_GUESTS - 1, HI_GUESTS):
        for j in range(TESTS_PER_GUEST):
            f = open(f'test/guest-{i + 1}/test-{i + 1}-{j}.txt', 'w')
            output_files.append(f)
            processes.append(subprocess.Popen(['./hotel', f'{i + 1}'], stdout=f, cwd=os.getcwd()))
    
    for f in output_files:
        f.close()

    counter = 0
    for p in processes:
        p.wait()
        counter += 1
    
    print(f'\t{counter} simulations ran')

def validate_output(out: list[str], num_guests: int):
    out = [l.rstrip() for l in out]

    i = 0
    n = len(out)

    # Validate simulation starts
    if out[i] != 'Simulation starts':
        raise Exception(f'Line {i}: {out[i]}\n\tSimulation did not start\n')

    i += 1
    desk_employee_count = 2
    bellhop_count = 2
    guest_count = num_guests
    
    guest_ids = set()
    
    # Validate everyone has been created
    for _ in range(num_guests + 4):
        line = out[i].split(' ')
        
        person = None
        id = None
        trail = None
        if len(line) == 3:
            person = line[0]
            id = int(line[1])
            trail = line[2]
        elif len(line) == 5:
            person = ' '.join(line[:3])
            id = int(line[3])
            trail = line[4]
        else:
            raise Exception(f'Line {i}: {out[i]}\n\tPerson created statement does not fit format\n')

        if person == DESK_EMPLOYEE:
            desk_employee_count -= 1
            if desk_employee_count < 0:
                raise Exception(f'Line {i}: {out[i]}\n\tToo many front desk employees\n')
        elif person == BELLHOP:
            bellhop_count -= 1
            if bellhop_count < 0:
                raise Exception(f'Line {i}: {out[i]}\n\tToo many bellhop employees\n')
        elif person == GUEST:
            guest_ids.add(id)
            guest_count -= 1
            if guest_count < 0:
                raise Exception(f'Line {i}: {out[i]}\n\tToo many guests\n')
        else:
            raise Exception(f'Line {i}: {out[i]}\n\tPerson created statement does not fit format\n')
        
        if trail != 'created':
            raise Exception(f'Line {i}: {out[i]}\n\tPerson created statement does not fit format\n')

        i += 1
    
    if len(guest_ids) != num_guests:
        raise Exception(f'Line {i}: {out[i]}\n\tGuest ids not unique\n')

    # Everyone has been created, validate simulation
    guest_states = [[CREATION, -1] for _ in range(num_guests)]
    
    while i < n - 1:
        try:
            line = out[i].split(' ')
            person = line[0]

            if person == GUEST:
                id = int(line[1])

                # Join
                if line[2] == 'joined':
                    if guest_states[id][0] != RETIRE:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {id} in incorrect state {guest_states[id][0]}\n')
                    guest_states[id][0] = JOIN

                # Enter hotel
                elif line[3] == 'hotel':
                    if guest_states[id][0] != CREATION:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {id} in incorrect state {guest_states[id][0]}\n')
                    guest_states[id][0] = ENTER_HOTEL
                    guest_states[id][1] = int(line[5])
                
                # Receive room key
                elif line[2] == 'receives' and line[3] == 'room':
                    if guest_states[id][0] != REGISTER:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {id} in incorrect state {guest_states[id][0]}\n')
                    guest_states[id][0] = RECEIVE_KEY

                # Request help
                elif line[2] == 'requests':
                    if guest_states[id][0] != RECEIVE_KEY:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {id} in incorrect state {guest_states[id][0]}\n')
                    if guest_states[id][1] <= 2:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {id} does not need bag help {guest_states[id][1]}\n')
                    guest_states[id][0] = REQUEST_HELP

                # Enter room
                elif line[3] == 'room':
                    if (guest_states[id][1] <= 2 and guest_states[id][0] != RECEIVE_KEY) or (guest_states[id][1] > 2 and guest_states[id][0] != BELLHOP_RECEIVE):
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {id} in incorrect state {guest_states[id][0]}\n')
                    guest_states[id][0] = ENTER_ROOM

                # Receive bags
                elif line[2] == 'receives':
                    if guest_states[id][0] != ENTER_ROOM:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {id} in incorrect state {guest_states[id][0]}\n')
                    guest_states[id][0] = RECEIVE_BAGS

                # Retire
                elif line[2] == 'retires':
                    if (guest_states[id][1] <= 2 and guest_states[id][0] != ENTER_ROOM) or (guest_states[id][1] > 2 and guest_states[id][0] != RECEIVE_BAGS):
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {id} in incorrect state {guest_states[id][0]}\n')
                    guest_states[id][0] = RETIRE

                else:
                    raise Exception(f'Line {i}: {out[i]}\n\tUnrecognized line\n')

            elif person == FRONT:
                guest_id = int(line[6])

                # Register guest
                if line[4] == 'registers':
                    if guest_states[guest_id][0] != ENTER_HOTEL:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {guest_id} in incorrect state {guest_states[guest_id][0]}\n')
                    guest_states[guest_id][0] = REGISTER
            elif person == BELLHOP:
                guest_id = int(line[6])

                # Receive bags
                if line[2] == 'receives':
                    if guest_states[guest_id][1] <= 2:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {guest_id} does not need bag help\n')
                    if guest_states[guest_id][0] != REQUEST_HELP:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {guest_id} in incorrect state {guest_states[guest_id][0]}\n')
                    guest_states[guest_id][0] = BELLHOP_RECEIVE

                # Deliver bags
                elif line[2] == 'delivers':
                    if guest_states[guest_id][0] != ENTER_ROOM:
                        raise Exception(f'Line {i}: {out[i]}\n\tGuest {guest_id} in incorrect state {guest_states[guest_id][0]}\n')
                    guest_states[guest_id][0] = BELLHOP_DELIVER
            else:
                raise
        except Exception as e:
            print(e)
            raise Exception(f'Line {i}: {out[i]}\n')
        i += 1

    # Validate simulation ends
    if out[-1] != 'Simulation ends':
        raise Exception(f'Line {n - 1}: {out[-1]}\n\tSimulation did not end\n')

if __name__ == '__main__':
    LO_GUESTS = 1
    HI_GUESTS = 25
    TESTS_PER_GUEST = 50

    out_folder = 'test'

    clean_folder(out_folder)

    run_simulations(LO_GUESTS, HI_GUESTS, TESTS_PER_GUEST)
    
    print('Validating output')
    counter = 0
    for i in range(LO_GUESTS, HI_GUESTS + 1):
        for j in range(TESTS_PER_GUEST):
            out_file = f'test/guest-{i}/test-{i}-{j}.txt'
            
            try:
                with open(out_file, 'r') as f:
                    validate_output(f.readlines(), i)
            except Exception as e:
                print(str(e))
                raise Exception(f'Test {out_file} failed\n')
            
            counter += 1
        
    print(f'\t{counter} tests passed')



