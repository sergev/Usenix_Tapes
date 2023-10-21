#include "local.h"

if ! -r cc.old				cp CC_PATH cc.old
if ! -r c0.old				cp C0_PATH c0.old
if ! -r c1.old				cp C1_PATH c1.old
if ! -r c2.old				cp C2_PATH c2.old
if ! -r f0.old -a -r F0_PATH		cp F0_PATH f0.old
if ! -r f1.old -a -r F1_PATH		cp F1_PATH f1.old

cp cc CC_PATH
cp c0 C0_PATH
cp c1 C1_PATH
cp c2 C2_PATH
if -r F0_PATH				cp f0 F0_PATH
if -r F1_PATH				cp f1 F1_PATH
