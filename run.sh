#!/bin/bash

rpm -q admc-test &>/dev/null || (echo "Package admc-test not installed, please install it." && exit 1)

# kinit administrator

temp_file=$(mktemp)

count_all=$(rpm -ql admc-test | grep -c admc_test)
count_cur=1

test_list=("admc_test_upn_edit" "admc_test_unlock_edit" "admc_test_ad_interface" "admc_test_ad_security")
# test_list=("admc_test_upn_edit" "admc_test_bool_attribute_dialog" "admc_test_bool_attribute_dialog" "admc_test_unlock_edit")
# test_list=("admc_test_ad_interface" "admc_test_upn_edit" "admc_test_bool_attribute_dialog" "admc_test_bool_attribute_dialog" "admc_test_unlock_edit")

containsElement () {
    local e match="$1"
    shift
    for e; do [[ "$e" == "$match" ]] && return 0; done
    return 1
}

for _test in ${test_list[@]};
do
    echo -e "\n*** [${count_cur}/${count_all}] test ${_test} ***"

    # sleep 0.1

    _test_path="admc/build/${_test}"

    if ${_test_path}; then
        echo "PASS: ${_test}" >>${temp_file}
    else
        echo "FAIL: ${_test}" >>${temp_file}
    fi
    count_cur=$((++count_cur))
done

echo -e "\n*** RESULTS ***"
cat ${temp_file} | sort

