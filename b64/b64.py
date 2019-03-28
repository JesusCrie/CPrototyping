import pprint

TRANSLATION_TABLE = list(range(ord('A'), ord('Z') + 1)) + list(range(ord('a'), ord('z') + 1)) + list(range(ord('0'), ord('9') + 1)) + [ord('+'), ord('/')]
COMPLEMENT = ord('=')


def gen_table():
    return list(range(ord('A'), ord('Z') + 1)) + list(range(ord('a'), ord('z') + 1)) + list(range(ord('0'), ord('9') + 1)) + [ord('+'), ord('/')]


def encode(ptext: str):
    ptext = [(ptext[x:x+3]) for x in range(0, len(ptext), 3)]
    if not len(ptext[-1]) == 3:
        last = ptext[-1]
        ptext[-1] = ([0 for c in range(len(last), last + (3 - last % 3))])
        for i in range(len(ptext[-1], 3)):
            # nan lol
            pass

    for i, c in enumerate(ptext):
        merger |= (c << (i*8))
        print(bin(merger))

if __name__ == "__main__":
    print(encode("Hi!"))