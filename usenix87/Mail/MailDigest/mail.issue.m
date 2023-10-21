
# Front end for mailing out the digest...

            vol=volume_number

echo "Mailing \"$digest_name, Volume `cat $vol`\""

exec nohup mail.digest
